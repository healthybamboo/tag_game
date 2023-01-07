PHONY: test clean

TEST_BUILD_DIR := test/build


test: $(TEST_BUILD_DIR)
# unit tests
	set -x && cd $(TEST_BUILD_DIR) && cmake ../ && make && ./testCommunication && ./testGame && ./testUtils

$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

clean:
	rm -rf $(TEST_BUILD_DIR)

PHONY: buildup

BUILD_DIR := build

buildup: $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)
	set -x && cd $(BUILD_DIR) && cmake ../src && make 