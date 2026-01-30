#include "entrance.h"

Eina_List *
entrance_theme_themes_get(void)
{
   Eina_Iterator *themes;
   Eina_List *targets = NULL;
   Eina_File_Direct_Info *file_stat;

   themes = eina_file_stat_ls(PACKAGE_DATA_DIR"/themes/");
   if (!themes)
     return NULL;
   EINA_ITERATOR_FOREACH(themes, file_stat)
     {
        const char *basename;
        basename = eina_str_split(&file_stat->path[file_stat->name_start], ".",2)[0];
        if (basename[0] != '.'
            && file_stat->type == EINA_FILE_REG
            && eina_str_has_extension(file_stat->path, ".edj"))
          targets = eina_list_append(targets, eina_stringshare_add(basename));
     }
   eina_iterator_free(themes);
   return targets;
}

