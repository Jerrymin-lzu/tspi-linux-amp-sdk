# Rockchip RT-Thread Documents Buiding Guide

## 安装mkdocs

```shell
$ wget https://bootstrap.pypa.io/get-pip.py
$ python get-pip.py
$ pip install mkdocs
$ mkdocs --version
mkdocs, version 1.0.4 from /home/cmc/.local/lib/python2.7/site-packages/mkdocs (Python 2.7)
```

## 下载

   本文档工程是为了适配我司的 RT-Thread 工程（后简称 RTT ），其构建脚本对 RTT 的路径有依赖，所以请确保在 RTT 根目录去下载本工程。

```shell
$ cd /path/to/rtt
$ git clone "ssh://git@10.10.10.29:29418/rtos/rt-thread/Rockchip-docs" && scp -p -P 29418 git@10.10.10.29:hooks/commit-msg "Rockchip-docs/.git/hooks/"  #请把git换成你的用户名
$ ls -l
drwxrwxr-x  5 cmc cmc 4096 Mar  3 10:42 Rockchip-docs
```

## 替换ID策略

   mkdocs 是调用 python-markdown 来把源文件转成 HTML 文档的，其 ID 生成策略和 typora 不太一样，不支持中文，这就导致只有中文的 header 生成的 ID 变成 ‘_x' 这样的格式（其中 x 是递增的数字），这就给跳转带来麻烦，所以我们加了一个中文支持，具体修改方法如下：

```shell
# 修改 ～/.local/lib/python2.7/site-packages/markdown/extensions/toc.py
diff --git a/markdown/extensions/toc.py b/markdown/extensions/toc.py
index 8f2b13f..5541e58 100644
--- a/markdown/extensions/toc.py
+++ b/markdown/extensions/toc.py
@@ -21,6 +21,10 @@ import re
 import unicodedata
 import xml.etree.ElementTree as etree

+def origin(value, separator):
+    """ Using the origin text, to make it URL friendly. """
+    value = re.sub(r'[^\w\s-]', '', value).strip().lower()
+    return re.sub(r'[%s\s]+' % separator, separator, value)

 def slugify(value, separator):
     """ Slugify a string, to make it URL friendly. """
@@ -137,7 +141,10 @@ class TocTreeprocessor(Treeprocessor):
         self.marker = config["marker"]
         self.title = config["title"]
         self.base_level = int(config["baselevel"]) - 1
-        self.slugify = config["slugify"]
+        if callable(config["slugify"]):
+            self.slugify = config["slugify"]
+        else:
+            self.slugify = eval(config["slugify"])
         self.sep = config["separator"]
         self.use_anchors = parseBoolValue(config["anchorlink"])
         self.anchorlink_class = config["anchorlink_class"]
```

   需要注意的是，虽然支持中文，但是 URL 本身是不支持一些特殊字符，如：'.'、'\n' 等，所以最终你的 header 到 ID 还是有一个转换的过程，规则如下：

- Step1：除了字母、数字、下划线、连接符和空格，其他全部删掉

- Step2：转成小写

- Step3：所有的空格都被替换成 '-'，注意多个连续空格会被替换成一个

下面是几个具体例子：

| Header             | ID          |
| ------------------ | ----------- |
| 1. 标题1           | 1-1         |
| 1. header1 标题1   | 1-header1-1 |
| 1.1.1 Header1 标题 | 111-header1 |

   要跳转到其他章节就需要了解这种转换关系，引用的时候按正确的 ID 填写，例如你要跳转到同一个文档中标题为：'1.1.1 Header1 标题' 的章节，就需要用：'#111-header1' 来引用。最后生成 HTML 文档时，请检查每个跳转的是否有效。

## 生成

```shell
./build.sh soc_name                        #soc_name即你要生成的soc名字，如：rk2108
```

## 浏览文档

   生成的文档是 html 格式，所以用浏览器打开 ./site/index.html 即可。