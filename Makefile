CXX ?= g++

TARGET := qrMqtt
BUILD_DIR := build
TEST_TARGET := $(BUILD_DIR)/qrmqtt_tests
SRC_DIR := src
INC_DIR := include

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

OPENCV_PKG := $(shell pkg-config --exists opencv4 && printf opencv4 || (pkg-config --exists opencv && printf opencv))
OPENCV_CFLAGS := $(shell [ -n "$(OPENCV_PKG)" ] && pkg-config --cflags $(OPENCV_PKG))
OPENCV_LIBS := $(shell [ -n "$(OPENCV_PKG)" ] && pkg-config --libs $(OPENCV_PKG))
OPENSSL_LIBS := $(shell pkg-config --libs openssl)

CPPFLAGS += -I$(INC_DIR) $(OPENCV_CFLAGS)
CXXFLAGS ?= -Wall -Wextra -g -std=c++11
DEPFLAGS := -MMD -MP
LDLIBS += $(OPENCV_LIBS) -lmosquitto -lzbar $(OPENSSL_LIBS)

.PHONY: all clean check-deps test

all: check-deps $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): tests/core_tests.cpp src/config.cpp src/duplicate_filter.cpp src/event.cpp src/token_validator.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(OPENSSL_LIBS)

check-deps:
	@pkg-config --exists opencv4 || pkg-config --exists opencv || { echo "Missing OpenCV development package. Install libopencv-dev."; exit 1; }
	@printf '#include <mosquitto.h>\nint main(){return 0;}\n' | $(CXX) -x c++ - -lmosquitto -o /tmp/qrMqtt-mosq-check >/dev/null 2>&1 || { echo "Missing Mosquitto development package. Install libmosquitto-dev."; exit 1; }
	@printf '#include <zbar.h>\nint main(){return 0;}\n' | $(CXX) -x c++ - -lzbar -o /tmp/qrMqtt-zbar-check >/dev/null 2>&1 || { echo "Missing ZBar development package. Install libzbar-dev."; exit 1; }
	@pkg-config --exists openssl || { echo "Missing OpenSSL development package. Install libssl-dev."; exit 1; }
	@rm -f /tmp/qrMqtt-mosq-check /tmp/qrMqtt-zbar-check

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)

