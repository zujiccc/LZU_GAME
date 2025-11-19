#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
DAIR-V2X 数据集完整使用示例
包含数据加载、预处理、可视化等完整流程
"""

import os
import numpy as np
import json
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import open3d as o3d
from v2x.dataset.dair_v2x_for_detection import DAIRV2XI, DAIRV2XV, VICDataset
from v2x.dataset.dataset_utils import load_json, read_pcd, read_jpg
from v2x_utils import id_to_str

class DAIRV2XDataLoader:
    """DAIR-V2X数据集加载器"""
    
    def __init__(self, data_root, split="train", sensor_type="lidar"):
        """
        初始化数据加载器
        
        Args:
            data_root: 数据集根目录
            split: 数据分割类型 ('train', 'val', 'test')
            sensor_type: 传感器类型 ('lidar', 'camera')
        """
        self.data_root = data_root
        self.split = split
        self.sensor_type = sensor_type
        
        # 数据集路径配置
        self.split_data_path = os.path.join(data_root, "data/split_datas/cooperative-split-data.json")
        
        # 初始化数据集
        self.inf_dataset = DAIRV2XI(
            path=data_root,
            args=self._get_args(),
            split=split,
            sensortype=sensor_type
        )
        
        self.veh_dataset = DAIRV2XV(
            path=data_root,
            args=self._get_args(),
            split=split,
            sensortype=sensor_type
        )
        
        self.vic_dataset = VICDataset(
            path=data_root,
            args=self._get_args(),
            split=split,
            sensortype=sensor_type
        )
    
    def _get_args(self):
        """获取配置参数"""
        class Args:
            split_data_path = self.split_data_path
            model = "late_fusion"  # 默认模型类型
        return Args()
    
    def load_infrastructure_data(self, idx):
        """加载基础设施端数据"""
        frame, labels, filter_obj = self.inf_dataset[idx]
        return {
            'frame': frame,
            'labels': labels,
            'filter': filter_obj,
            'pointcloud': self._load_pointcloud(frame['pointcloud_path']) if self.sensor_type == 'lidar' else None,
            'image': self._load_image(frame['image_path']) if self.sensor_type == 'camera' else None
        }
    
    def load_vehicle_data(self, idx):
        """加载车辆端数据"""
        frame, labels, filter_obj = self.veh_dataset[idx]
        return {
            'frame': frame,
            'labels': labels,
            'filter': filter_obj,
            'pointcloud': self._load_pointcloud(frame['pointcloud_path']) if self.sensor_type == 'lidar' else None,
            'image': self._load_image(frame['image_path']) if self.sensor_type == 'camera' else None
        }
    
    def load_cooperative_data(self, idx):
        """加载协同数据"""
        vic_frame, labels, filter_obj = self.vic_dataset[idx]
        
        # 加载基础设施数据
        inf_frame = vic_frame.inf_frame
        inf_data = {
            'frame': inf_frame,
            'pointcloud': self._load_pointcloud(inf_frame['pointcloud_path']) if self.sensor_type == 'lidar' else None,
            'image': self._load_image(inf_frame['image_path']) if self.sensor_type == 'camera' else None
        }
        
        # 加载车辆数据
        veh_frame = vic_frame.veh_frame
        veh_data = {
            'frame': veh_frame,
            'pointcloud': self._load_pointcloud(veh_frame['pointcloud_path']) if self.sensor_type == 'lidar' else None,
            'image': self._load_image(veh_frame['image_path']) if self.sensor_type == 'camera' else None
        }
        
        return {
            'vic_frame': vic_frame,
            'labels': labels,
            'filter': filter_obj,
            'infrastructure': inf_data,
            'vehicle': veh_data
        }
    
    def _load_pointcloud(self, pcd_path):
        """加载点云数据"""
        try:
            points, _ = read_pcd(pcd_path)
            return points
        except Exception as e:
            print(f"加载点云失败: {e}")
            return None
    
    def _load_image(self, img_path):
        """加载图像数据"""
        try:
            return read_jpg(img_path)
        except Exception as e:
            print(f"加载图像失败: {e}")
            return None


class DAIRV2XDataAnalyzer:
    """DAIR-V2X数据分析器"""
    
    def __init__(self):
        self.class_names = ["Car", "Truck", "Van", "Bus", "Pedestrian", 
                           "Cyclist", "Tricyclist", "Motorcyclist", "Barrowlist", "TrafficCone"]
    
    def analyze_dataset_statistics(self, loader):
        """分析数据集统计信息"""
        stats = {
            'total_samples': len(loader.inf_dataset) + len(loader.veh_dataset),
            'infrastructure_samples': len(loader.inf_dataset),
            'vehicle_samples': len(loader.veh_dataset),
            'cooperative_samples': len(loader.vic_dataset),
            'class_distribution': {},
            'pointcloud_stats': {}
        }
        
        # 统计类别分布
        for i in range(min(100, len(loader.inf_dataset))):  # 采样前100个样本
            data = loader.load_infrastructure_data(i)
            for view in ['camera', 'lidar']:
                if view in data['labels']:
                    labels = data['labels'][view]
                    for label in labels.labels:
                        class_name = label['type']
                        stats['class_distribution'][class_name] = stats['class_distribution'].get(class_name, 0) + 1
        
        return stats
    
    def visualize_pointcloud_with_labels(self, pointcloud, labels, title="Point Cloud Visualization"):
        """可视化点云和标注框"""
        if pointcloud is None:
            print("没有点云数据可供可视化")
            return
        
        # 创建Open3D点云对象
        pcd = o3d.geometry.PointCloud()
        pcd.points = o3d.utility.Vector3dVector(pointcloud[:, :3])
        
        # 如果有强度信息，设置颜色
        if pointcloud.shape[1] >= 4:
            colors = plt.cm.viridis(pointcloud[:, 3] / pointcloud[:, 3].max())
            pcd.colors = o3d.utility.Vector3dVector(colors[:, :3])
        
        # 创建可视化窗口
        vis = o3d.visualization.Visualizer()
        vis.create_window(window_name=title)
        vis.add_geometry(pcd)
        
        # 添加3D边界框
        if labels and hasattr(labels, 'labels'):
            for label in labels.labels:
                if '3d_location' in label and '3d_dimensions' in label:
                    box = self._create_3d_box(label)
                    vis.add_geometry(box)
        
        vis.run()
        vis.destroy_window()
    
    def _create_3d_box(self, label):
        """创建3D边界框"""
        center = [label['3d_location']['x'], label['3d_location']['y'], label['3d_location']['z']]
        dimensions = [label['3d_dimensions']['l'], label['3d_dimensions']['w'], label['3d_dimensions']['h']]
        rotation = label.get('rotation', 0)
        
        # 创建边界框
        box = o3d.geometry.OrientedBoundingBox(center, np.eye(3), dimensions)
        box.rotate(o3d.geometry.get_rotation_matrix_from_xyz([0, 0, rotation]))
        box.color = [1, 0, 0]  # 红色
        
        return box


class DAIRV2XDataProcessor:
    """DAIR-V2X数据预处理器"""
    
    def __init__(self):
        self.voxel_size = 0.1
        self.point_cloud_range = [-75.2, -75.2, -2, 75.2, 75.2, 4]
    
    def preprocess_pointcloud(self, points):
        """预处理点云数据"""
        if points is None:
            return None
        
        # 1. 范围过滤
        mask = self._filter_points_by_range(points)
        filtered_points = points[mask]
        
        # 2. 下采样
        downsampled_points = self._voxel_downsample(filtered_points)
        
        # 3. 强度归一化
        if downsampled_points.shape[1] >= 4:
            downsampled_points[:, 3] = downsampled_points[:, 3] / 255.0
        
        return downsampled_points
    
    def _filter_points_by_range(self, points):
        """按范围过滤点云"""
        x_range = (points[:, 0] >= self.point_cloud_range[0]) & (points[:, 0] <= self.point_cloud_range[3])
        y_range = (points[:, 1] >= self.point_cloud_range[1]) & (points[:, 1] <= self.point_cloud_range[4])
        z_range = (points[:, 2] >= self.point_cloud_range[2]) & (points[:, 2] <= self.point_cloud_range[5])
        
        return x_range & y_range & z_range
    
    def _voxel_downsample(self, points):
        """体素下采样"""
        # 简化的体素下采样实现
        voxel_indices = np.floor(points[:, :3] / self.voxel_size).astype(np.int32)
        _, unique_indices = np.unique(voxel_indices, axis=0, return_index=True)
        
        return points[unique_indices]
    
    def filter_labels_by_range(self, labels, point_range):
        """按范围过滤标注"""
        if not labels or not hasattr(labels, 'labels'):
            return labels
        
        filtered_labels = []
        for label in labels.labels:
            if '3d_location' in label:
                loc = label['3d_location']
                if (point_range[0] <= loc['x'] <= point_range[3] and
                    point_range[1] <= loc['y'] <= point_range[4] and
                    point_range[2] <= loc['z'] <= point_range[5]):
                    filtered_labels.append(label)
        
        labels.labels = filtered_labels
        return labels


def main():
    """主函数 - 完整使用示例"""
    
    # 1. 配置参数
    data_root = "data/DAIR-V2X"  # 数据集根目录
    split = "train"  # 使用训练集
    sensor_type = "lidar"  # 使用激光雷达数据
    
    print("=== DAIR-V2X 数据集使用示例 ===")
    print(f"数据根目录: {data_root}")
    print(f"数据分割: {split}")
    print(f"传感器类型: {sensor_type}")
    
    # 2. 初始化数据加载器
    print("\n1. 初始化数据加载器...")
    loader = DAIRV2XDataLoader(data_root, split, sensor_type)
    
    # 3. 数据分析
    print("\n2. 数据集统计分析...")
    analyzer = DAIRV2XDataAnalyzer()
    stats = analyzer.analyze_dataset_statistics(loader)
    
    print(f"总样本数: {stats['total_samples']}")
    print(f"基础设施样本数: {stats['infrastructure_samples']}")
    print(f"车辆样本数: {stats['vehicle_samples']}")
    print(f"协同样本数: {stats['cooperative_samples']}")
    print(f"类别分布: {stats['class_distribution']}")
    
    # 4. 加载示例数据
    print("\n3. 加载示例数据...")
    
    # 加载基础设施数据
    if len(loader.inf_dataset) > 0:
        inf_data = loader.load_infrastructure_data(0)
        print(f"基础设施数据加载成功 - 点云点数: {inf_data['pointcloud'].shape[0] if inf_data['pointcloud'] is not None else 0}")
    
    # 加载车辆数据
    if len(loader.veh_dataset) > 0:
        veh_data = loader.load_vehicle_data(0)
        print(f"车辆数据加载成功 - 点云点数: {veh_data['pointcloud'].shape[0] if veh_data['pointcloud'] is not None else 0}")
    
    # 加载协同数据
    if len(loader.vic_dataset) > 0:
        vic_data = loader.load_cooperative_data(0)
        print(f"协同数据加载成功")
    
    # 5. 数据预处理
    print("\n4. 数据预处理...")
    processor = DAIRV2XDataProcessor()
    
    if inf_data['pointcloud'] is not None:
        processed_points = processor.preprocess_pointcloud(inf_data['pointcloud'])
        print(f"预处理后的点云点数: {processed_points.shape[0] if processed_points is not None else 0}")
    
    # 6. 数据可视化
    print("\n5. 数据可视化...")
    try:
        if inf_data['pointcloud'] is not None and inf_data['labels'] is not None:
            analyzer.visualize_pointcloud_with_labels(
                inf_data['pointcloud'], 
                inf_data['labels']['lidar'],
                "基础设施点云可视化"
            )
    except Exception as e:
        print(f"可视化失败: {e}")
    
    # 7. 批量处理示例
    print("\n6. 批量处理示例...")
    batch_size = 5
    for i in range(min(batch_size, len(loader.inf_dataset))):
        data = loader.load_infrastructure_data(i)
        if data['pointcloud'] is not None:
            processed = processor.preprocess_pointcloud(data['pointcloud'])
            print(f"样本 {i}: 原始点数 {data['pointcloud'].shape[0]} -> 预处理后 {processed.shape[0] if processed is not None else 0}")
    
    print("\n=== 示例完成 ===")


if __name__ == "__main__":
    main()