GTEST_HEADERS = .
GTEST_LIBS = ../libs/gtest/libgtest.a
GLOG_HEADERS = .
GLOG_LIBS = ../libs/glog/libglog.a
CFLAGS = -lpthread -lrt
TARGET = bloom_filter_unittest
all:
	g++ -o $(TARGET)\
            bloom_filter_unittest.cc\
            murmur_hasher.cc\
            $(GTEST_LIBS)\
            $(GLOG_LIBS)\
            $(CFLAGS)\
            -I $(GTEST_HEADERS)\
            -I $(GLOG_HEADERS)
clean:
	rm -f $(TARGET)
