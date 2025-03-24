# Dify API Plugin for Unreal Engine

[![UE5.4 Compatible](https://img.shields.io/badge/Unreal%20Engine-5.4+-%25232E3438.svg?style=flat&logo=unrealengine)](https://www.unrealengine.com/)

[![Alpha Version](https://img.shields.io/badge/Version-0.1.0_Alpha-orange)](https://semver.org/)

**Dify API**是一个在Unreal Engine工程中使用Dify应用的插件，当前版本适用于`聊天助手`，已在UE5.4版本完成基础验证。

## 📂 分支管理

| 分支名称    | 说明                                         |
| :---------- | :------------------------------------------- |
| master      | 稳定发布分支，包含经过测试的核心功能         |
| development | 开发分支，可能存在未完成的功能或实验性代码   |
| intro-page  | 文档资源分支，存放README所需的图片等媒体文件 |

> ⚠️ **当前状态**：插件处于Alpha开发阶段，控制台会输出详细调试日志，建议使用时手动删除插件内的UE_LOG

## 🛠️ 安装指南

### 基础安装

1. 在项目根目录创建插件文件夹：
   `MyProject/Plugins/`

2. 通过以下方式获取插件源码：

   ```bash
   # 使用Git安装
   cd MyProject/Plugins/
   git clone -b master https://github.com/agentland-academy/DifyAPI-For-UnrealEngine.git
   ```

   或直接以ZIP压缩包的方式下载并解压到插件文件夹。
   最终插件目录格式：`MyProject/Plugins/DifyAPI-For-UnrealEngine`

3. 重新生成项目文件：

   - 右键`.uproject`文件 > Generate Visual Studio project files
   - 启动引擎时选择"Rebuild"

### 版本兼容性

| Unreal Engine 版本 | 支持状态             |
| :----------------- | :------------------- |
| 5.5.x及以上        | ⚠️ 需手动编译，未验证 |
| 5.4.x              | ✅ 完全支持           |
| 5.3.x及以下        | ⚠️ 需手动编译，未验证 |

## 🔍  使用指南——聊天助手

### 组件添加

1. 在任意目标Actor的组件面板中搜索添加`Dify Chat`组件
   ![AddDifyChatComponent.jpg](https://github.com/agentland-academy/DifyAPI-For-UnrealEngine/blob/Intro-Page/Imgs/AddDifyChatComponent.jpg?raw=true)

2. 通过以下任一方式初始化：
   
   **方式一：属性面板配置**

   选择Dify Chat组件，在右侧属性面板中直接填写。

   ![DifyChatProperty.jpg](https://github.com/agentland-academy/DifyAPI-For-UnrealEngine/blob/Intro-Page/Imgs/DifyChatProperty.jpg?raw=true)
   
   **方式二：蓝图初始化节点**

   使用组件内的`InitDifyChat`节点。

   ![DifyChatInit.jpg](https://github.com/agentland-academy/DifyAPI-For-UnrealEngine/blob/Intro-Page/Imgs/DifyChatInit.jpg?raw=true)

### 参数说明

- **Dify URL**：Dify的链接，格式为http://xxx.xxx.xxx.xxx/v1/chat-messages
- **Dify APIKey**：Dify应用的密钥，格式为app-xxxxx，如果要以字符串的方式保存记得加密工程
- **ChatName**：这个组件所扮演的角色的名字，**只在引擎内使用**
- **UserName**：跟这个组件对话的用户的名字，**只在引擎内使用**
- **Dify Chat Type**：选择Single Chat是单次对话，没有记忆。选择Multi Chat是多轮对话，有记忆。
- **Dify Chat Response Mode**：blocking是一次性接收所有回复，streaming是像打字机一样流式接收。
- **Dify Inputs**：如果Dify应用中有额外的输入，就使用`DifyChatInputs`结构体组成数组传入。
- **Conversation ID**：不常用，可以设置为之前的ID来继续之前的对话。留空时自动生成新会话ID

> 这些参数都可以随时单独设置

### 事件系统

**Talk to AI**

使用插件与`聊天助手`的Dify应用对话十分简单，只需使用组件内的`Talk to AI`节点。其中Message引脚是要传入的对话内容。

![DifyChatTalkToAI.jpg](https://github.com/agentland-academy/DifyAPI-For-UnrealEngine/blob/Intro-Page/Imgs/DifyChatTalkToAI.jpg?raw=true)

**事件分发器/委托**

当前组件内有三个`事件分发器/委托`，对应`响应时`、`响应后`和`对话后`的事件。只需点击属性面板里的+号就能在当前Actor里创建绑定事件，或者通过蓝图手动绑定。

![DifyChatEvents.jpg](https://github.com/agentland-academy/DifyAPI-For-UnrealEngine/blob/Intro-Page/Imgs/DifyChatEvents.jpg?raw=true)

**On Dify Chat Talk To`对话后`**

向Dify发送信息后立即触发`On Dify Chat Talk To`事件，其中参数为：

- **UserName**：当前组件内所保存的UserName变量，**只在引擎内使用**
- **Message**：向Dify所发送的内容

![OnDifyChatTalkTo.jpg](https://github.com/agentland-academy/DifyAPI-For-UnrealEngine/blob/Intro-Page/Imgs/OnDifyChatTalkTo.jpg?raw=true)

**On Dify Chat Responding`响应时`**

每次收到响应片段时触发`On Dify Chat Responding`事件，blocking模式下触发一次，streaming模式触发多次。其中参数为一个结构体，包含所返回的所有数据。其中常用的有：

- **Conversation Id** ：本次对话所生成或使用的Conversation ID
- **Answer** ：Dify所返回的内容
- **Chat Name**： 当前组件所扮演的角色的名字，**只在引擎内使用**

![OnDifyChatResponding.jpg](https://github.com/agentland-academy/DifyAPI-For-UnrealEngine/blob/Intro-Page/Imgs/OnDifyChatResponding.jpg?raw=true)

**On Dify Chat Responded`响应后`**

响应结束后触发一次`On Dify Chat Responded`事件，没有返回的参数。

![OnDifyChatResponded.jpg](https://github.com/agentland-academy/DifyAPI-For-UnrealEngine/blob/Intro-Page/Imgs/OnDifyChatResponded.jpg?raw=true)

## 🤖 示例

![GIF_Test01.gif](https://github.com/agentland-academy/DifyAPI-For-UnrealEngine/blob/Intro-Page/Imgs/GIF_Test01.gif?raw=true)

