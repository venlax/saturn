BUILD_DIR = build

THREADS = 8  

all: build


build:
	if [ ! -d "$(BUILD_DIR)" ]; then mkdir $(BUILD_DIR); fi
	cd $(BUILD_DIR) && make -j$(THREADS)
	cd ..
run:
	make build
	./build/test_$(arg)
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all build build_parallel clean
