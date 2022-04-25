# encoding:utf-8
import gc
import matplotlib.pyplot as plt
import matplotlib.colors as col
# from intervals import IntInterval
import PIL.Image as Image
from enum import Enum
import numpy as np
import zipfile
import math
from osgeo import gdal
import os
import io


def read_file(root_path, ext_filter):
    """读取文件夹中的文件路径"""
    files_name = []
    for root, dirs, files in os.walk(root_path):
        for files_path in files:
            m_file_path = os.path.join(root, files_path)
            extension = os.path.splitext(m_file_path)[1][1:]
            if ext_filter and extension in ext_filter:
                files_name.append(m_file_path)
    return files_name


def transparent_back(img):
    img = img.convert('RGBA')
    L, H = img.size
    color_0 = (255,255,255,255) #要透明色
    for h in range(H):
        for l in range(L):
            dot = (l,h)
            color_1 = img.getpixel(dot)
            if color_1 == color_0:
                color_1 = color_1[:-1] + (0,)
                img.putpixel(dot,color_1)
    return img


def extract_file(z_file, path):
    """解压zip文件"""
    z = zipfile.ZipFile(z_file, "r")
    for p in z.namelist():
        z.extract(p, path)
    z.close()


def extract_zip(zip_file_name):
    zip_list = read_file(zip_file_name, ".zip")
    tiff_path = []
    for zip_file in zip_list:
        (file_path, temp_filename) = os.path.split(zip_file)
        shot_name = os.path.splitext(temp_filename)[0]  # 分离文件与扩展名
        remain = file_path.replace(zip_file_name, "")
        m_path = out_path + remain
        if not os.path.exists(m_path):
            os.makedirs(m_path)
        # 解压文件
        zip_tif = m_path + "/" + shot_name + ".tif"
        if not os.path.isfile(m_path):
            extract_file(zip_file, m_path)

        tiff_path.append(zip_tif)
    return tiff_path


def read_tif(tif_file_name):
    tif_list = read_file(tif_file_name, ".tif")
    return tif_list


def custom_def_colorbar(type):
    data_list = [-999]
    data_list.append(-998)
    t = 0
    for t in range(0,255,8):
        if (type == "chl"):
            m_data = math.pow(10, (t / 72.75)) / 100
        elif (type == "sdd"):
            m_data = math.pow(10, (t / 143.41))
        elif (type == "ssc"):
            m_data = math.pow(10, (t / 68.94)) / 10
        elif (type == "sal"):
            m_data = t/16 + 20
        elif (type == "tau"):
            m_data = t/255
        elif (type == "dic"):
            m_data = 2*t+1800
        elif (type == "doc"):
            m_data = t/3.2+36
        elif (type == "npp"):
            m_data = t*5
        elif (type == "pic"):
            m_data =  math.pow(10, (3.7*t/255-1))
        elif (type == "poc"):
            m_data = t/12.8+40
        elif (type == "ta"):
            m_data = t+2050
        elif (type == "tsm"):
            m_data = math.pow(10, (3 * t / 255 - 1))
        elif (type == "cdom"):
            m_data = math.pow(10, (t / 127.5)) / 200
        elif (type == "cpom660"):
            m_data = math.pow(10, (t / 196)) / 20
        elif (type == "nap"):
            m_data = math.pow(10, (t/127.5)) / 200
        elif (type == "par"):
            m_data = t / 4
        elif (type == "sss"):
            m_data = 2 / 63 * t + 27
        elif (type == "sst"):
            m_data = 1 / 8 * t - 2

        data_list.append(m_data)
    return data_list


def get_color_schemes(color_r_list, color_g_list, color_b_list):
    list_color = []
    for i in range(len(color_r_list)):
        m_tuple = (color_r_list[i] * 0.0 / 255, color_g_list[i] * 0.0 / 255, color_b_list[i] * 0.0 / 255)
        list_color.append(m_tuple)
    return list_color


# 读取文件
count = 1
for f_line in io.open("rendererPath.txt", "r", encoding='UTF-8'):
    f_line = f_line.rstrip(u"\n").strip()
    if (f_line.find("#") == -1):
        # 数据目录
        if (count == 1):
            file_name = f_line.replace("\\", "/")
        # 解压目录
        if (count == 2):
            out_path = f_line.replace("\\", "/")
        # 类型
        if (count == 3):
            type = f_line
            # 类型
        if (count == 4):
            index = f_line
        count += 1
# 数据目录
# file_name = ur"C:/Users/Administrator/Desktop/ESACCI_SAT_MERGE_19980901TO19980930_L3B_GLOBAL_4KM_CHL_OCCCIV31"
# out_path = ur"C:/Users/Administrator/Desktop/n_data"
# 解压数据
# tif_path = read_tif(file_name)
tif_path = extract_zip(file_name)

# 定义颜色条
#b2r
color_list = [(0, 0, 0), (0, 0, 255.0 / 255), (0, 33.0 / 255, 255.0 / 255), (0, 67.0 / 255, 255.0 / 255),
              (0, 101.0 / 255, 255.0 / 255),
              (0, 131.0 / 255, 255.0 / 255), (0, 165.0 / 255, 255.0 / 255), (0, 199.0 / 255, 255.0 / 255),
              (0, 233.0 / 255, 255.0 / 255), (0, 255.0 / 255, 255.0 / 255), (0, 255.0 / 255, 246.0 / 255),
              (0, 255.0 / 255, 212.0 / 255), (0, 255.0 / 255, 178.0 / 255), (0, 255.0 / 255, 114.0 / 255),
              (0, 255.0 / 255, 114.0 / 255), (0, 255.0 / 255, 80.0 / 255), (0, 255.0 / 255, 46.0 / 255),
              (17.0 / 255, 255.0 / 255, 12.0 / 255), (51.0 / 255, 255.0 / 255, 0), (85.0 / 255, 255.0 / 255, 0),
              (119.0 / 255, 255.0 / 255, 0), (148.0 / 255, 255.0 / 255, 0),
              (182.0 / 255, 255.0 / 255, 0), (216.0 / 255, 255.0 / 255, 0), (250.0 / 255, 255.0 / 255, 0),
              (255.0 / 255, 229.0 / 255, 0), (255.0 / 255, 195.0 / 255, 0), (255.0 / 255, 161.0 / 255, 0),
              (255.0 / 255, 127.0 / 255, 0), (255.0 / 255, 97.0 / 255, 0), (255.0 / 255, 63.0 / 255, 0),
              (255.0 / 255, 29.0 / 255, 0), (255.0 / 255, 0, 0), (255.0 / 255, 0, 0)]
#r2b
color_list1 = [(0, 0, 0), (255.0 / 255, 0, 0), (255.0 / 255, 29.0 / 255, 0), (255.0 / 255, 63.0 / 255, 0), (255.0 / 255, 97.0 / 255, 0), (255.0 / 255, 127.0 / 255, 0), (255.0 /255, 161.0 / 255, 0),
(255.0 / 255, 195.0 / 255, 0), (255.0 / 255, 229.0 / 255, 0), (250.0 / 255, 255.0 / 255, 0), (216.0 / 255, 255.0 / 255, 0), (182.0 / 255, 255.0 / 255, 0), (148.0 / 255,255.0 / 255, 0), (119.0 / 255, 255.0 / 255, 0),
(85.0 / 255, 255.0 / 255, 0), (51.0 / 255, 255.0 / 255, 0), (17.0 / 255, 255.0 / 255, 12.0 / 255), (0, 255.0 / 255, 46.0 / 255), (0, 255.0 / 255,114.0 / 255), (0, 255.0 / 255, 80.0 / 255), (0, 255.0 / 255, 114.0 / 255),
(0, 255.0 / 255, 178.0 / 255), (0, 255.0 / 255, 212.0 / 255), (0, 255.0 / 255, 246.0 / 255), (0, 255.0 / 255, 255.0 /255), (0, 233.0 / 255, 255.0 / 255), (0, 199.0 / 255, 255.0 / 255), (0, 165.0 / 255, 255.0 / 255),
(0, 131.0 / 255, 255.0 / 255), (0, 101.0 / 255, 255.0 / 255), (0, 67.0 / 255, 255.0 / 255),(0, 33.0 / 255, 255.0 / 255), (0, 0, 255.0 / 255)]


bounds = custom_def_colorbar(type)
# sst颜色条
if (index == "b2r"):
    c_map = col.LinearSegmentedColormap.from_list('bwb', color_list)
    c_norm = col.BoundaryNorm(bounds, c_map.N)
# 颜色条
else:
    c_map = col.LinearSegmentedColormap.from_list('bwb', color_list1)
    c_norm = col.BoundaryNorm(bounds, c_map.N)
# 出图
for tif in tif_path:
    ds = gdal.Open(tif)
    geo_transform = ds.GetGeoTransform()
    bands = ds.RasterCount
    for band in range(bands):
        band += 1
        fig = plt.gcf()
        plt.ioff()
        band_data = ds.GetRasterBand(band)
        m_data = band_data.ReadAsArray()
        # m_data[m_data < -999] = -999
        data = np.ma.masked_values(m_data, -9999)
        print(m_data.max())
        # 去除图像周围的白边
        height, width = data.shape
        # 如果dpi=300，那么图像大小=height*width
        fig.set_size_inches(width / 100.0 / 3.0, height / 100.0 / 3.0)
        im = plt.imshow(data, cmap=c_map, norm=c_norm)
        plt.axis('off')
        # plt.colorbar()
        # plt.show()
        png_path = tif.replace(".tif", ".png")
        plt.gca().xaxis.set_major_locator(plt.NullLocator())
        plt.gca().yaxis.set_major_locator(plt.NullLocator())
        plt.subplots_adjust(top=1, bottom=0, left=0, right=1, hspace=0, wspace=0)
        plt.margins(0, 0)
        plt.savefig(png_path, dpi=300)
    plt.close('all')
    del ds, band, data, m_data
    gc.collect()
    img=Image.open(png_path)
    img=transparent_back(img)
    img.save(png_path)
    os.remove(tif)
