#include <ctype.h>
#include <unistd.h>

#include "entrance_client.h"

static char *_distro = NULL;
static char *_distro_id = NULL;
static char *_distro_logo = NULL;
static char *_logo_path = NULL;

static char *
_entrance_system_value_get(const char *value)
{
   const char *start;
   const char *end;
   size_t len;

   if (!value) return NULL;

   start = value;
   while (*start && isspace((unsigned char)*start))
     start++;
   if (!*start) return NULL;

   end = start + strlen(start);
   while ((end > start) && isspace((unsigned char)*(end - 1)))
     end--;

   if (((end - start) >= 2) &&
       (((*start == '"') && (*(end - 1) == '"')) ||
        ((*start == '\'') && (*(end - 1) == '\''))))
     {
        start++;
        end--;
     }

   len = end - start;
   if (!len) return NULL;

   return eina_strndup(start, len);
}

static void
_entrance_system_value_set(char **dst, const char *value)
{
   if (*dst) return;
   *dst = _entrance_system_value_get(value);
}

static Eina_Bool
_entrance_system_icon_name_valid(const char *name)
{
   const char *p;

   if (!name || !name[0]) return EINA_FALSE;

   for (p = name; *p; p++)
     {
        if (!isalnum((unsigned char)*p) &&
            (*p != '-') && (*p != '_') && (*p != '.'))
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_entrance_system_logo_lookup(const char *name)
{
   static const unsigned int sizes[] = { 128, 96, 64, 48, 32 };
   const char *themes[2];
   unsigned int theme_count = 0;
   unsigned int i;
   unsigned int j;

   if (!_entrance_system_icon_name_valid(name))
     return EINA_FALSE;

   themes[theme_count++] = elm_config_icon_theme_get();
   if (!themes[0] || strcmp(themes[0], "hicolor"))
     themes[theme_count++] = "hicolor";

   for (i = 0; i < theme_count; i++)
     {
        const char *path;

        if (!themes[i]) continue;
        for (j = 0; j < sizeof(sizes) / sizeof(sizes[0]); j++)
          {
             path = efreet_icon_path_find(themes[i], name, sizes[j]);
             if (path)
               {
                  _logo_path = strdup(path);
                  return _logo_path ? EINA_TRUE : EINA_FALSE;
               }
          }
     }

   return EINA_FALSE;
}

static Eina_Bool
_entrance_system_logo_lookup_id(const char *id)
{
   char name[256];
   int len;

   if (!_entrance_system_icon_name_valid(id))
     return EINA_FALSE;

   len = snprintf(name, sizeof(name), "distributor-logo-%s", id);
   if ((len >= 0) && ((size_t)len < sizeof(name)) &&
       _entrance_system_logo_lookup(name))
     return EINA_TRUE;

   len = snprintf(name, sizeof(name), "start-here-%s", id);
   if ((len >= 0) && ((size_t)len < sizeof(name)) &&
       _entrance_system_logo_lookup(name))
     return EINA_TRUE;

   return EINA_FALSE;
}

static void
_entrance_system_distro_detect(void)
{
   FILE *f;
   char line[512];
   char *name = NULL;

   f = fopen("/etc/os-release", "r");
   if (!f)
     f = fopen("/usr/lib/os-release", "r");

   if (!f)
     {
        _distro = strdup("Linux");
        return;
     }

   while (fgets(line, sizeof(line), f))
     {
        char key[64];
        char value[448];

        if (sscanf(line, "%63[^=]=%447[^\n]", key, value) != 2)
          continue;

        if (!strcmp(key, "NAME"))
          _entrance_system_value_set(&name, value);
        else if (!strcmp(key, "ID"))
          _entrance_system_value_set(&_distro_id, value);
        else if (!strcmp(key, "LOGO"))
          _entrance_system_value_set(&_distro_logo, value);
     }
   fclose(f);

   _distro = name ? name : strdup(_distro_id ? _distro_id : "Linux");
}

static void
_entrance_system_logo_detect(void)
{
   _logo_path = NULL;

   if (!elm_need_efreet())
     return;

   if (_entrance_system_logo_lookup(_distro_logo))
     return;
   if (_entrance_system_logo_lookup_id(_distro_id))
     return;
}

static void
_entrance_system_init(void)
{
   char hostname[256] = "localhost";

   if (gethostname(hostname, sizeof(hostname)) == 0)
     {
        char *dot;

        hostname[sizeof(hostname) - 1] = '\0';
        dot = strchr(hostname, '.');
        if (dot) *dot = '\0';
     }

   _entrance_system_distro_detect();
   _entrance_system_logo_detect();

   PT("System: %s on %s, logo: %s",
      _distro ? _distro : "unknown",
      hostname,
      _logo_path ? _logo_path : "none");
}

const char *
entrance_system_distro_get(void)
{
   if (!_distro)
     _entrance_system_init();

   return _distro;
}

const char *
entrance_system_logo_get(void)
{
   if (!_distro)
     _entrance_system_init();

   return _logo_path;
}

void
entrance_system_shutdown(void)
{
   free(_distro);
   free(_distro_id);
   free(_distro_logo);
   free(_logo_path);
   _distro = NULL;
   _distro_id = NULL;
   _distro_logo = NULL;
   _logo_path = NULL;
}
