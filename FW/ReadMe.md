# 源代码下载

由于本源代码包含第三方源代码,故直接下载可能有部分源代码缺失，需要通过以下方法解决:

- 在进行git clone 使用--recurse-submodules参数。

- 若已通过git clone下载,则在源代码目录中执行以下命令下载子模块:

  ```bash
   git submodule update --init --recursive
  ```

若源代码未完全下载，将不能正常编译。

# 开发环境

默认情况下，采用[esp-idf](https://github.com/espressif/esp-idf.git).

进入固件目录之前，需要先进入esp-idf 环境。

- 进入[esp-idf](3rdparty/esp-idf)目录,若目录为空则表示源代码未下载完成。
- 安装工具(执行install脚本)。
- 导出工具(调用export脚本，注意不是执行脚本（如`bash`下应当使用`source`命令，`cmd`下应当使用`call`命令）)
- 检查`idf.py`是否可执行，若无法执行则检查上述步骤是否成功。
- 进入固件目录（根据实际需要选择对应的目录）
- 使用`idf.py build` 构建固件，使用`idf.py flash`烧录固件，使用`idf.py monitor`观察固件日志输出。

# 目录说明

- [数字]-[名称]：数字为固件编号，名称为固件名称。
- [3rdparty](3rdparty):第三方源代码。

# 固件列表

- [001-base](001-base):基础模板。

