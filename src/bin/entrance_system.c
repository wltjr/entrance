#include "entrance_client.h"
#include <sys/utsname.h>
#include <unistd.h>

static char *_hostname = NULL;
static char *_distro = NULL;
static char *_logo_path = NULL;

/**
 * Detect distro from /etc/os-release
 */
static void
_entrance_system_distro_detect(void)
{
   FILE *f;
   char line[256];
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
        char key[64], value[128];
        
        if (sscanf(line, "%63[^=]=%127[^\n]", key, value) == 2)
          {
             /* Remove quotes */
             char *v = value;
             if (*v == '"') v++;
             char *end = v + strlen(v) - 1;
             if (*end == '"') *end = '\0';
             
             if (!strcmp(key, "NAME"))
               {
                  name = strdup(v);
                  break;
               }
          }
     }
   fclose(f);
   
   _distro = name ? name : strdup("Linux");
}

/**
 * Find distro logo in standard paths
 */
static void
_entrance_system_logo_detect(void)
{
   const char *paths[] = {
      "/usr/share/icons/hicolor/128x128/apps/",
      "/usr/share/icons/hicolor/64x64/apps/",
      "/usr/share/icons/Papirus/64x64/apps/",
      "/usr/share/icons/Papirus/48x48/apps/",
      "/usr/share/icons/Vertex/places/symbolic/",
      "/usr/share/pixmaps/"
   };
   const char *exts[] = {".svg", ".png", ""};
   const char *icon_name = NULL;
   
   if (!_distro)
     {
        _logo_path = NULL;
        return;
     }
   
   /* Map distro name to icon name */
   if (strstr(_distro, "Gentoo"))
     icon_name = "distributor-logo-gentoo";
   else if (strstr(_distro, "Ubuntu"))
     icon_name = "ubuntu";
   else if (strstr(_distro, "Debian"))
     icon_name = "debian";
   else if (strstr(_distro, "Arch"))
     icon_name = "archlinux";
   else if (strstr(_distro, "Fedora"))
     icon_name = "fedora";
   else if (strstr(_distro, "openSUSE"))
     icon_name = "opensuse";
   else
     icon_name = "distributor-logo";
   
   /* Search for logo */
   for (size_t i = 0; i < sizeof(paths)/sizeof(paths[0]); i++)
     {
        for (int j = 0; j < (int)(sizeof(exts)/sizeof(exts[0])); j++)
          {
             char path[512];
             snprintf(path, sizeof(path), "%s%s%s", paths[i], icon_name, exts[j]);
             
             if (ecore_file_exists(path))
               {
                  _logo_path = strdup(path);
                  return;
               }
          }
     }
   
   _logo_path = NULL;
}

/**
 * Initialize system detection
 */
static void
_entrance_system_init(void)
{
   char hostname[256];
   
   /* Get hostname */
   if (gethostname(hostname, sizeof(hostname)) == 0)
     {
        char *dot = strchr(hostname, '.');
        if (dot) *dot = '\0';
        _hostname = strdup(hostname);
     }
   else
     _hostname = strdup("localhost");
   
   /* Detect distro and logo */
   _entrance_system_distro_detect();
   _entrance_system_logo_detect();
   
   PT("System: %s on %s, logo: %s", 
      _distro ? _distro : "unknown",
      _hostname ? _hostname : "unknown",
      _logo_path ? _logo_path : "none");
}

/**
 * Get system hostname (lazy init)
 */
const char *
entrance_system_hostname_get(void)
{
   if (!_hostname)
     _entrance_system_init();
   
   return _hostname;
}

/**
 * Get distro name (lazy init)
 */
const char *
entrance_system_distro_get(void)
{
   if (!_distro)
     _entrance_system_init();
   
   return _distro;
}

/**
 * Get distro logo path (lazy init)
 */
const char *
entrance_system_logo_get(void)
{
   if (!_distro)  /* Use distro as init flag */
     _entrance_system_init();
   
   return _logo_path;
}
