#define FUSE_USE_VERSION 31
#define _FILE_OFFSET_BITS 64

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <curl/curl.h>
#include <cjson/cJSON.h> 

// Replace these with your GitHub repository details
#define GITHUB_USERNAME "isabelabuzzo"
#define GITHUB_REPO "est_dados"
#define GITHUB_TOKEN "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"

#define API_URL "https://api.github.com/repos/" GITHUB_USERNAME "/" GITHUB_REPO "/contents"

// Struct to hold libcurl response data
struct MemoryStruct {
    char *memory;
    size_t size;
};

// libcurl write callback function to write received data to a buffer
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        // Out of memory
        fprintf(stderr, "Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}


static int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void)offset;
    (void)fi;

    // Initialize libcurl
    CURL *curl_handle = curl_easy_init();
    if (!curl_handle) {
        fprintf(stderr, "Error initializing libcurl\n");
        return -ENOENT; // Error initializing libcurl
    }

    // Set libcurl options for GitHub API authentication and response handling
    struct curl_slist *headers = NULL;
    char auth_header[100];
    snprintf(auth_header, sizeof(auth_header), "Authorization: token %s", GITHUB_TOKEN);
    headers = curl_slist_append(headers, auth_header);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);

    struct MemoryStruct chunk;
    chunk.memory = malloc(1); // Allocate initial memory
    chunk.size = 0;
    curl_easy_setopt(curl_handle, CURLOPT_URL, API_URL);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode res = curl_easy_perform(curl_handle);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl_handle);
        free(chunk.memory);
        return -ENOENT; // Error performing libcurl request
    }

    // Parse JSON response using cJSON
    cJSON *root = cJSON_Parse(chunk.memory);
    if (!root) {
        fprintf(stderr, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        curl_easy_cleanup(curl_handle);
        free(chunk.memory);
        return -ENOENT; // Error parsing JSON
    }

    // Extract file and directory names from JSON and fill FUSE buffer
    cJSON *current = root->child;
    while (current != NULL) {
        if (strcmp(current->string, "name") == 0) {
            filler(buf, current->valuestring, NULL, 0); // Fill FUSE buffer with file/folder names
        }
        current = current->next;
    }

    // Clean up cJSON and libcurl resources
    cJSON_Delete(root);
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);

    return 0;
}

static struct fuse_operations myfs_oper = {
    .readdir = myfs_readdir,
    
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &myfs_oper, NULL);
}

