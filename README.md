
libv4l2cpp
====================

It is a C++ wrapper for V4L2

Dependencies
------------
 - liblog4cpp5-dev (optional)
 
V4L2 Capture
-------------
 - create a V4L2 Capture interface using MMAP interface:

         V4L2DeviceParameters param("/dev/video0", V4L2_PIX_FMT_*, width, height, fps, verbose);
         V4l2Capture* videoCapture = V4l2Capture::create(param, V4l2Access::IOTYPE_MMAP);

 - data are available :

         timeval timeout; 
         bool isReadable = (videoCapture->isReadable(&timeout) == 1);

 - read data :

         size_t nb = videoCapture->read(buffer, bufferSize);


V4L2 Output
-------------

 - To create a V4L2 Output interface using MMAP interface:

         V4L2DeviceParameters param("/dev/video0", V4L2_PIX_FMT_*, width, height, fps, verbose);
         V4l2Output* videoOutput = V4l2Output::create(param, V4l2Access::IOTYPE_MMAP);

 - data could be written :

         timeval timeout; 
         bool isWritable = (videoOutput->isWritable(&timeout) == 1);

 - write data :

         size_t nb = videoOutput->write(buffer, bufferSize);
