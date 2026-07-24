#ifndef ENTRANCE_FILL_H
#define ENTRANCE_FILL_H

typedef char *(*Entrance_Fill_Text_Get_Func) (void *data, Evas_Object *obj, const char *part);
typedef Evas_Object *(*Entrance_Fill_Content_Get_Func) (void *data, Evas_Object *obj, const char *part);
typedef Eina_Bool (*Entrance_Fill_Cb_Func) (void *data, Elm_Object_Item *it);

typedef struct Entrance_Fill_
{
   const char *item_style; //maybe need to be provided by theme ?
   struct
     {
        Entrance_Fill_Text_Get_Func text_get;
        Entrance_Fill_Content_Get_Func content_get;
        Evas_Smart_Cb sel;
        void *data;
     } func;
   Elm_Genlist_Item_Class *glc;
   Elm_Gengrid_Item_Class *ggc;
} Entrance_Fill;

Entrance_Fill *entrance_fill_new(const char *item_style,
                                 Entrance_Fill_Text_Get_Func label_get,
                                 Entrance_Fill_Content_Get_Func content_get);
void entrance_fill(Evas_Object *obj, Entrance_Fill *egf, const Eina_List *contents, Entrance_Fill_Cb_Func fill_cb, Evas_Smart_Cb func, void *data);
void entrance_fill_del(Entrance_Fill *ef);


#endif /* ENTRANCE_FILL_H */
