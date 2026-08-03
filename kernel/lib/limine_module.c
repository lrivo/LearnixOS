#include <learnix/lib/limine_module.h>
#include <learnix/lib/string.h>

__attribute ((
    used,
    section (".limine_requests"))) static volatile struct limine_module_request
    module_request = { .id = LIMINE_MODULE_REQUEST_ID, .revision = 5 };

struct limine_file*
limine_module_get(const char *path)
{
  struct limine_module_response *modules = module_request.response;
  struct limine_file *file;

  for (uint64_t i = 0; i < modules->module_count; i++)
  {
    file = modules->modules[i];
    if (strcmp(path, file->path) == 0)
      return file;
  }

  return 0;
}
