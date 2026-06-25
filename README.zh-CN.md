# aria2 自定义版说明

**简体中文** | [English](README.rst)

本仓库基于原版 aria2 源码进行二次开发，并新增了删除任务时可选同时删除文件的 RPC 扩展。其余功能保持与 upstream 一致。

## RPC 扩展：删除任务同时删除文件

为以下 JSON-RPC 方法新增可选布尔参数，用于控制是否删除已下载的文件和控制文件（*.aria2）：

- `aria2.remove(gid, removeFiles)`
- `aria2.forceRemove(gid, removeFiles)`
- `aria2.removeDownloadResult(gid, removeFiles)`

行为说明：
- `removeFiles=true`：删除任务/结果的同时尝试删除对应下载文件和控制文件。
- 省略或 `false`：行为与原版 aria2 相同，仅删除任务/结果，不删除文件。

示例请求（删除任务并删除文件）：

```json
{
  "jsonrpc": "2.0",
  "id": "example-remove",
  "method": "aria2.remove",
  "params": ["token:YOUR_SECRET", "2089b05ecca3d829", true]
}
```

## 构建提示（与原版一致）
构建依赖、配置选项与官方 aria2 保持一致，可参考 `README.rst` 的依赖与构建章节。完成 `autoreconf -i` 后，通常流程为：

```bash
./configure --with-openssl --with-libxml2
make -j4
# 可选：make check
```

如需使用 RPC 扩展，请确保客户端按上述新增参数调用相应方法。原有调用方式保持兼容。
