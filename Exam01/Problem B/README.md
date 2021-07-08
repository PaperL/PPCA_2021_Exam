# 使用说明

使用windows的同学推荐在wsl下完成这道题目。当然你也可以修改脚本的一些地方来适配windows环境。

`compress.hpp` 是你需要完成的文件。

`judge.cpp` 是你需要编译运行的文件，从标准输入读入某个`in`文件，会调用你的压缩和解压函数并输出PSNR，压缩文件大小以供参考。同时也会将你还原出的RGB值输出到`out.txt`供`rev.py`调试使用。

`rev.py` 调试用文件，需要安装`opencv-python`库。

无法安装的同学可以尝试 [在线可视化工具](http://82.156.197.136/)

```shell
pip3 install opencv-python
```
可以读取`judge.cpp`输出的`out.txt`并将其还原成`save.png`的图片，你可以直观地看到自己的压缩效果。

大致使用方法：

```shell
g++ -O2 judge.cpp -o judge
./judge < data/3.in
python3 rev.py
```

如果你希望用自己的图片来调试，`cast_into_data.py`可以将图片（png，jpg等格式都支持）生成输入的in文件。

提交时，请将==`compress.hpp`==内的所有内容粘贴至提交框。

