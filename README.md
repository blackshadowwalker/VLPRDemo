VLPRDemo
========
车牌识别Demo

用FFmpeg打开视频，并获取图像真图片进行车牌识别


[2014.2.10 17:20:04]
SVN R2
1.完成用FFmpeg打开并获取视频流数据
2.调用算法库，并输出识别车牌
3.可以显示但是失真，需要修改
下一步：
1.修改显示失真问题
2.完善界面输出
3.增加配置

[2014.2.11 16:44:39]
1. 正常显示图片，不再失真
2. 车牌识别调用成功，并用3个线程来处理，1）播放线程;2）处理结果线程，包含存储图片车车牌图片并显示；3)识别线程
下一步
1. 将识别过程拆分成 视频流获取线程（存入队列）和识别线程，以提供速度
2. 修改界面

[2014.2.11 18:13:11]
SVN R5
1.已将将识别过程拆分成 视频流获取线程（存入队列）和识别线程，以提供速度
下一步
1.将结果显示在界面，并增加配置界面

[2014.2.11 19:16]
SVN R6
修改了部分界面，将车牌识别结果显示在m_list
下一步:
增加配置界面

[2014.2.13 12:00]
SVN R7
1. 增加了打开图片并进行车牌识别,正常运行
2. 修改视频流车牌识别，现在速度很快，修改部分如下
	* VideoThread 与 RecognitionThread
	* VideoThread 与 PlayThread
	* RecognitionThread 与 ProcessResultThread
	的线程访问冲突，现在修改成了
	* VideoThread 和 RecognitionThread 访问队列 imagesQueue,VideoThread一直new imageData内存空间，然后放入队列 imagesQueue,
	  RecognitionThread 从队列 imagesQueue 获取数据并delete
	* RecognitionThread将显示的图片（每次memcpy到全局的内存空间imageDataForShow）放入队列 imagesQueuePlay,
	  然后接着处理车牌，PlayThread一直读取队列imagesQueuePlay

下一步：
1. 图片识别的增加一下张按钮功能	

[2014.2.13 14:03]
SVN R8
图片识别的增加一下张按钮功能	

[2014.2.13 16:50]
SVN  R9
stable
next:
use new TFLPRecognition.dll


[2014.2.13 19:03]
SVN  R10
1. 使用新版本的车牌识别 dll 
next
1.配置界面
2.用sqlite 进入数据保存

[2014.2.14 12:05:11]
SVN R11
添加了快速处理功能，缺点是会丢帧

[2014.2.14 16:02:28]
SVN R12
增加了配置界面，基本属于完工






