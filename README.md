
libv4l2cpp
====================

It is C++ wrapper for V4L2

License
------------
Domain public 

Dependencies
------------
 - liblog4cpp5-dev
 
V4L2 Capture
-------------
 - create a V4L2 Capture interface using MMAP interface:

         V4L2DeviceParameters param("/dev/video0", V4L2_PIX_FMT_*, width, height, fps, verbose);
         V4l2Capture* videoCapture = V4l2DeviceFactory::CreateVideoCapture(param, V4l2DeviceFactory::IOTYPE_MMAP);

 - read data :

         size_t nb = videoCapture->read(buffer, bufferSize);


V4L2 Output
-------------

 - To create a V4L2 Output interface using MMAP interface:

         V4L2DeviceParameters param("/dev/video0", V4L2_PIX_FMT_*, width, height, fps, verbose);
         V4l2Output* videoOutput = V4l2DeviceFactory::CreateVideoOutput(param, V4l2DeviceFactory::IOTYPE_MMAP);

 - write data :

         size_t nb = videoCapture->write(buffer, bufferSize);
