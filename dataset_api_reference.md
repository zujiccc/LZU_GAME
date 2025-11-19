# DAIR-V2X 数据集API接口参考文档

## 概述

DAIR-V2X（Dataset for AI Research - Vehicle to Everything）是清华大学智能产业研究院（AIR）发布的大规模车路协同自动驾驶数据集。本文档提供完整的数据集API接口说明和使用指南。

## 数据集结构

### 1. 数据集组成

DAIR-V2X包含三个主要子数据集：

- **DAIR-V2X-I**：基础设施端数据集（10,084帧）
- **DAIR-V2X-V**：车辆端数据集（22,325帧）  
- **DAIR-V2X-C**：车路协同数据集（38,845帧）

### 2. 文件组织结构

```
data/
├── single-infrastructure-side/          # DAIR-V2X-I
│   ├── image/                          # 图像数据
│   ├── velodyne/                       # 点云数据
│   ├── calib/                          # 标定文件
│   └── label/                          # 标注文件
├── single-vehicle-side/               # DAIR-V2X-V
│   ├── image/
│   ├── velodyne/
│   ├── calib/
│   └── label/
└── cooperative-vehicle-infrastructure/  # DAIR-V2X-C
    ├── infrastructure-side/
    ├── vehicle-side/
    └── cooperative/
```

## 核心API接口

### 1. 数据集类

#### DAIRV2XI - 基础设施端数据集

```python
from v2x.dataset.dair_v2x_for_detection import DAIRV2XI

# 初始化
dataset = DAIRV2XI(
    path="data/DAIR-V2X",
    args=args,  # 配置参数对象
    split="train",  # 数据分割: train/val/test
    sensortype="lidar",  # 传感器类型: lidar/camera
    extended_range=None  # 扩展范围配置
)

# 获取数据样本
frame, labels, filter_obj = dataset[0]
```

**参数说明：**
- `path`: 数据集根目录路径
- `args`: 包含配置参数的对象，需包含 `split_data_path` 和 `model` 属性
- `split`: 数据分割类型，支持 `train`、`val`、`test`
- `sensortype`: 传感器类型，支持 `lidar` 和 `camera`
- `extended_range`: 扩展范围过滤，用于限制数据范围

#### DAIRV2XV - 车辆端数据集

```python
from v2x.dataset.dair_v2x_for_detection import DAIRV2XV

# 初始化
dataset = DAIRV2XV(
    path="data/DAIR-V2X",
    args=args,
    split="train",
    sensortype="lidar",
    extended_range=None
)
```

#### VICDataset - 车路协同数据集

```python
from v2x.dataset.dair_v2x_for_detection import VICDataset

# 初始化
dataset = VICDataset(
    path="data/DAIR-V2X",
    args=args,
    split="train", 
    sensortype="lidar",
    extended_range=None,
    val_data_path=""  # 验证数据路径（可选）
)
```

### 2. 数据帧类

#### InfFrame - 基础设施帧

```python
from v2x.dataset.dataset_utils import InfFrame

# 创建基础设施帧
inf_frame = InfFrame(path, data_info)

# 访问帧数据
pointcloud_path = inf_frame['pointcloud_path']
image_path = inf_frame['image_path']
batch_id = inf_frame['batch_id']
```

**主要属性：**
- `pointcloud_path`: 点云文件路径
- `image_path`: 图像文件路径
- `batch_id`: 批次ID，用于关联车路数据
- `calib_virtuallidar_to_camera_path`: 虚拟激光雷达到相机标定
- `calib_camera_intrinsic_path`: 相机内参标定

#### VehFrame - 车辆帧

```python
from v2x.dataset.dataset_utils import VehFrame

# 创建车辆帧
veh_frame = VehFrame(path, data_info)

# 访问帧数据
lidar_path = veh_frame['pointcloud_path']
image_path = veh_frame['image_path']
```

#### VICFrame - 车路协同帧

```python
from v2x.dataset.dataset_utils import VICFrame

# 创建车路协同帧
vic_frame = VICFrame(path, data_info, veh_frame, inf_frame, 0)

# 坐标转换
transform = vic_frame.transform(from_coord="Vehicle_lidar", to_coord="World")
```

### 3. 标注类

#### Label - 标注处理

```python
from v2x.dataset.dataset_utils import Label

# 创建标注对象
label = Label(label_path, filter_obj)

# 访问标注数据
boxes_3d = label['boxes_3d']  # 3D边界框
labels = label['labels']       # 类别标签
scores = label['scores']       # 置信度分数
```

### 4. 工具函数

#### 文件IO工具

```python
from v2x.dataset.dataset_utils import load_json, read_pcd, read_jpg

# 加载JSON文件
data = load_json(json_path)

# 读取点云文件
points, time = read_pcd(pcd_path)

# 读取图像文件
image = read_jpg(jpg_path)
```

#### 坐标转换工具

```python
from v2x_utils import get_trans, box_translation

# 获取坐标转换矩阵
trans_matrix = get_trans(from_coord, to_coord)

# 边界框坐标转换
transformed_boxes = box_translation(boxes, trans_matrix)
```

## 数据格式说明

### 1. 点云数据格式

点云数据采用PCD格式存储，每个点包含4个维度：
- x, y, z: 三维坐标
- intensity: 反射强度（0-255，归一化到0-1）

```python
# 点云数据形状: (N, 4)
points.shape  # (点数, 4)
```

### 2. 图像数据格式

图像数据采用JPEG格式存储，通过mmcv读取：

```python
image = read_jpg(image_path)  # shape: (H, W, 3)
```

### 3. 标注数据格式

#### 单视角标注

```json
{
  "type": "Car",                    // 类别名称
  "truncated_state": 0,             // 截断状态
  "occluded_state": 0,              // 遮挡状态
  "2d_box": {                       // 2D边界框
    "xmin": 100, "ymin": 200,
    "xmax": 300, "ymax": 400
  },
  "3d_dimensions": {                // 3D尺寸
    "h": 1.5, "w": 1.8, "l": 4.0
  },
  "3d_location": {                    // 3D位置
    "x": 10.0, "y": 2.0, "z": 0.5
  },
  "rotation": 0.1                     // 旋转角度
}
```

#### 协同标注

```json
{
  "type": "Car",                    // 类别名称
  "world_8_points": [...],          // 世界坐标系8个角点
  "system_error_offset": {          // 系统误差偏移
    "delta_x": 0.1, "delta_y": 0.2
  }
}
```

### 4. 标定数据格式

#### 相机内参

```json
{
  "cam_K": [[fx, 0, cx], [0, fy, cy], [0, 0, 1]],  // 内参矩阵
  "dist_coeffs": [k1, k2, p1, p2, k3]              // 畸变系数
}
```

#### 外参转换

```json
{
  "rotation": [[...], [...], [...]],  // 旋转矩阵
  "translation": [tx, ty, tz]         // 平移向量
}
```

## 使用示例

### 1. 基础数据加载

```python
import os
from v2x.dataset.dair_v2x_for_detection import DAIRV2XI, DAIRV2XV, VICDataset
from v2x.dataset.dataset_utils import load_json

# 配置参数
class Args:
    split_data_path = "data/split_datas/cooperative-split-data.json"
    model = "late_fusion"

args = Args()

# 加载基础设施数据集
inf_dataset = DAIRV2XI(
    path="data/DAIR-V2X",
    args=args,
    split="train",
    sensortype="lidar"
)

# 访问第一个样本
frame, labels, filter_obj = inf_dataset[0]
print(f"Frame ID: {frame['batch_id']}")
print(f"Labels count: {len(labels.labels)}")
```

### 2. 点云数据处理

```python
from v2x.dataset.dataset_utils import read_pcd
import numpy as np

# 读取点云
points, _ = read_pcd(frame['pointcloud_path'])
print(f"Point cloud shape: {points.shape}")

# 点云预处理
def preprocess_points(points, range_limit=75.0):
    # 范围过滤
    mask = np.abs(points[:, 0]) < range_limit
    mask &= np.abs(points[:, 1]) < range_limit
    filtered_points = points[mask]
    
    # 强度归一化
    filtered_points[:, 3] = filtered_points[:, 3] / 255.0
    
    return filtered_points

processed_points = preprocess_points(points)
```

### 3. 车路协同数据处理

```python
# 加载协同数据集
vic_dataset = VICDataset(
    path="data/DAIR-V2X",
    args=args,
    split="train",
    sensortype="lidar"
)

# 获取协同样本
vic_frame, labels, filter_obj = vic_dataset[0]

# 获取基础设施和车辆帧
inf_frame = vic_frame.inf_frame
veh_frame = vic_frame.veh_frame

# 坐标转换（车辆坐标系到世界坐标系）
transform = vic_frame.transform("Vehicle_lidar", "World")
world_points = transform(veh_points)
```

### 4. 标注数据处理

```python
from v2x.dataset.dataset_utils import Label
from v2x_utils import RectFilter

# 创建范围过滤器
extended_range = [[-75.2, -75.2, -2, 75.2, 75.2, 4]]
filter_obj = RectFilter(extended_range[0])

# 加载标注
label = Label(label_path, filter_obj)

# 访问标注信息
boxes_3d = label['boxes_3d']      # 3D边界框
class_names = label['labels']     # 类别名称

# 类别统计
class_counts = {}
for class_name in class_names:
    class_counts[class_name] = class_counts.get(class_name, 0) + 1

print(f"Class distribution: {class_counts}")
```

### 5. 数据分割处理

```python
# 加载分割配置
split_data = load_json("data/split_datas/cooperative-split-data.json")

# 获取训练集ID
train_ids = split_data["batch_split"]["train"]
val_ids = split_data["batch_split"]["val"]
test_ids = split_data["batch_split"]["test"]

print(f"Train samples: {len(train_ids)}")
print(f"Val samples: {len(val_ids)}")
print(f"Test samples: {len(test_ids)}")
```

## 参数配置详解

### 1. 数据集初始化参数

| 参数名 | 类型 | 说明 | 默认值 |
|--------|------|------|--------|
| path | str | 数据集根目录路径 | 必填 |
| args | object | 配置参数对象 | 必填 |
| split | str | 数据分割类型 | "train" |
| sensortype | str | 传感器类型 | "lidar" |
| extended_range | list | 扩展范围过滤 | None |

### 2. 配置参数对象

配置参数对象需包含以下属性：

```python
class Args:
    split_data_path = "path/to/split_data.json"  # 分割数据路径
    model = "late_fusion"  # 模型类型（影响数据处理）
```

### 3. 范围过滤参数

`extended_range` 参数格式：

```python
extended_range = [
    [x_min, y_min, z_min, x_max, y_max, z_max],  # 范围限制
    # 可以添加多个范围
]
```

## 常见使用场景

### 1. 模型训练数据准备

```python
def prepare_training_data(dataset, output_dir):
    """准备训练数据"""
    for i in range(len(dataset)):
        frame, labels, filter_obj = dataset[i]
        
        # 加载点云
        points, _ = read_pcd(frame['pointcloud_path'])
        
        # 预处理
        processed_points = preprocess_points(points)
        
        # 保存处理后的数据
        np.save(f"{output_dir}/points_{i}.npy", processed_points)
        
        # 保存标注
        label_data = {
            'boxes_3d': labels['boxes_3d'],
            'labels': labels['labels']
        }
        save_json(label_data, f"{output_dir}/labels_{i}.json")
```

### 2. 数据验证和检查

```python
def validate_dataset(dataset):
    """验证数据集完整性"""
    missing_files = []
    
    for i in range(len(dataset)):
        frame, labels, filter_obj = dataset[i]
        
        # 检查点云文件
        if not os.path.exists(frame['pointcloud_path']):
            missing_files.append(frame['pointcloud_path'])
        
        # 检查图像文件
        if not os.path.exists(frame['image_path']):
            missing_files.append(frame['image_path'])
        
        # 检查标注文件
        if not os.path.exists(labels.label_path):
            missing_files.append(labels.label_path)
    
    return missing_files
```

### 3. 批量数据转换

```python
def convert_to_kitti_format(dataset, output_dir):
    """转换为KITTI格式"""
    for i in range(len(dataset)):
        frame, labels, filter_obj = dataset[i]
        
        # 转换点云格式
        points, _ = read_pcd(frame['pointcloud_path'])
        save_bin(points, f"{output_dir}/velodyne/{i:06d}.bin")
        
        # 转换标注格式
        kitti_labels = convert_labels_to_kitti(labels)
        save_txt(kitti_labels, f"{output_dir}/label_2/{i:06d}.txt")
```

## 注意事项

1. **内存管理**：处理大规模点云数据时注意内存使用
2. **坐标系转换**：不同传感器使用不同的坐标系，需要正确转换
3. **时间同步**：车路协同数据需要考虑时间同步问题
4. **数据完整性**：使用前验证数据文件完整性
5. **性能优化**：批量处理时考虑并行化

## 故障排除

### 常见问题

1. **文件找不到错误**
   - 检查数据集路径是否正确
   - 验证数据文件是否存在
   - 检查文件权限

2. **内存不足错误**
   - 减少批量处理大小
   - 使用点云下采样
   - 增加系统内存

3. **坐标转换错误**
   - 验证标定文件完整性
   - 检查坐标系定义
   - 确认转换矩阵正确性

### 性能优化建议

1. **数据预加载**：提前加载常用数据
2. **缓存机制**：缓存处理后的数据
3. **并行处理**：使用多进程处理数据
4. **数据压缩**：使用压缩格式存储中间结果

## 更新日志

- v1.0.0: 初始版本，基础API接口
- v1.1.0: 增加车路协同支持
- v1.2.0: 优化性能和内存使用
- v1.3.0: 增加更多预处理工具

## 联系方式

如有问题，请联系：
- 邮箱：dair@air.tsinghua.edu.cn
- 项目地址：https://github.com/AIR-THU/DAIR-V2X