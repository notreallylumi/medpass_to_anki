#include <curl/curl.h>
#include <leptonica/allheaders.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tesseract/capi.h>
#include <unistd.h>

#define TEST_IMAGE "../test.png"

typedef struct {
  char *data;  // pointer to the allocated buffer
  size_t size; // current size of data stored (excluding null terminator)
} curl_buffer_t;

static size_t curl_write_cb(void *contents, size_t size, size_t nmemb,
                            void *user_data) {

  size_t num_bytes = size * nmemb;
  curl_buffer_t *buffer = (curl_buffer_t *)user_data;

  char *new_data = realloc(buffer->data, buffer->size + num_bytes + 1);
  if (!new_data)
    return 0;

  memcpy(new_data + buffer->size, contents, num_bytes + 1);
  buffer->data = new_data;
  buffer->size += num_bytes;
  buffer->data[buffer->size] = '\0';

  return num_bytes; // tell curl we handled all the bytes
}

char *generate_flashcards(const char *ocr_text) {
  const char *api_key = getenv("API_KEY");
  CURL *curl = curl_easy_init();
  if (!curl)
    return NULL;

  curl_buffer_t chunk = {.data = malloc(1), .size = 0};
  chunk.data[0] = 0;

  curl_easy_setopt(curl, CURLOPT_URL,
                   "https://api.groq.com/openai/v1/chat/completions");

  // Your Groq API key (hardcode for testing; better to read from env/config
  // later)
  char auth_header[256];
  snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s",
           api_key);

  struct curl_slist *headers =
      curl_slist_append(NULL, "Content-Type: application/json");
  headers = curl_slist_append(headers, auth_header);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  char *escaped_ocr = curl_easy_escape(curl, ocr_text, 0);
  if (!escaped_ocr) {
    return NULL;
  }
  // Build payload — Groq uses OpenAI-compatible format
  char payload[8192];
  snprintf(payload, sizeof(payload),
           "{"
           "\"model\": \"llama-3.1-8b-instant\","
           "\"messages\": ["
           "{\"role\": \"system\", \"content\": \"You are a medical flashcard "
           "expert. "
           "Convert the text to Anki flashcards. Output ONLY "
           "semicolon-separated lines. "
           "No extra text, no markdown, no explanations. Format: Front;Back\"},"
           "{\"role\": \"user\", \"content\": \"Text:\\n%s\"}"
           "],"
           "\"temperature\": 0.3,"
           "\"max_tokens\": 1200"
           "}",
           escaped_ocr);

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

  CURLcode res = curl_easy_perform(curl);

  char *result = NULL;
  if (res == CURLE_OK) {
    // Extract content (same as before)
    char *start = strstr(chunk.data, "\"content\":\"");
    if (start) {
      start += 11;
      char *end = strstr(start, "\"");
      if (end) {
        *end = '\0';
        result = strdup(start);
      }
    }
  }

  // Cleanup
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  free(chunk.data);

  return result;
}

int main(void) {
  TessBaseAPI *api = TessBaseAPICreate();
  if (TessBaseAPIInit3(api, NULL, "eng") != 0) {
    return -1;
  }
  PIX *image = pixRead(TEST_IMAGE);
  if (!image) {
    return -1;
  }
  TessBaseAPISetImage2(api, image);
  char *text = TessBaseAPIGetUTF8Text(api);
  char *cards = generate_flashcards(text);
  printf("%s\n", cards);
  pixDestroy(&image);
  TessBaseAPIEnd(api);
  TessBaseAPIDelete(api);
}
