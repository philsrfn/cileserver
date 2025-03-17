#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/file_ops.h"
#include "../include/logger.h"
#include "../include/config.h"

#define TEST_BUFFER_SIZE 1024

void test_file_write_read() {
    printf("Testing file write and read...\n");
    
    const char *test_data = "Hello, world! This is a test file.";
    const char *test_file = "test_file.txt";
    char read_buffer[TEST_BUFFER_SIZE];
    size_t bytes_read;
    
    // Write test file
    int result = write_file(test_file, test_data, strlen(test_data));
    assert(result == 0);
    
    // Read test file
    result = read_file(test_file, read_buffer, TEST_BUFFER_SIZE, &bytes_read);
    assert(result == 0);
    assert(bytes_read == strlen(test_data));
    
    // Verify content
    read_buffer[bytes_read] = '\0';
    assert(strcmp(read_buffer, test_data) == 0);
    
    // Clean up
    result = delete_file(test_file);
    assert(result == 0);
    
    printf("File write and read test passed!\n");
}

void test_directory_operations() {
    printf("Testing directory operations...\n");
    
    const char *test_dir = "test_dir";
    file_info_t entries[10];
    int num_entries;
    
    // Create directory
    int result = create_directory(test_dir);
    assert(result == 0);
    
    // Write a file in the directory
    char test_file_path[256];
    snprintf(test_file_path, sizeof(test_file_path), "%s/test.txt", test_dir);
    const char *test_data = "Test file in directory";
    result = write_file(test_file_path, test_data, strlen(test_data));
    assert(result == 0);
    
    // List directory
    result = list_directory(test_dir, entries, 10, &num_entries);
    assert(result == 0);
    assert(num_entries == 1);
    assert(strcmp(entries[0].name, "test.txt") == 0);
    
    // Get file info
    file_info_t info;
    result = get_file_info(test_file_path, &info);
    assert(result == 0);
    assert(strcmp(info.name, "test.txt") == 0);
    assert(info.size == strlen(test_data));
    assert(info.is_directory == 0);
    
    // Clean up
    result = delete_file(test_file_path);
    assert(result == 0);
    result = delete_file(test_dir);
    assert(result == 0);
    
    printf("Directory operations test passed!\n");
}

void test_path_validation() {
    printf("Testing path validation...\n");
    
    // Valid paths
    assert(is_path_valid("test.txt") == 1);
    assert(is_path_valid("dir/test.txt") == 1);
    assert(is_path_valid("/test.txt") == 1);
    
    // Invalid paths
    assert(is_path_valid("../test.txt") == 0);
    assert(is_path_valid("dir/../test.txt") == 0);
    assert(is_path_valid(NULL) == 0);
    assert(is_path_valid("") == 0);
    
    printf("Path validation test passed!\n");
}

int main() {
    // Initialize
    init_logger();
    load_config();
    init_file_ops();
    
    // Run tests
    test_file_write_read();
    test_directory_operations();
    test_path_validation();
    
    // Clean up
    cleanup_file_ops();
    cleanup_logger();
    
    printf("All tests passed!\n");
    return 0;
}