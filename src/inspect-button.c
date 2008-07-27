#include "parasite.h"


static void
on_inspect_widget(GtkWidget *grab_window, GdkEventButton *event,
                  ParasiteWindow *parasite)
{
   gdk_pointer_ungrab(event->time);
   gtk_widget_hide(parasite->highlight_window);

   if (parasite->selected_window != NULL)
   {
      GtkWidget *toplevel = NULL;
      GtkWidget *widget = NULL;

      gdk_window_get_user_data(
         gdk_window_get_toplevel(parasite->selected_window),
         (gpointer*)&toplevel);

      gdk_window_get_user_data(parasite->selected_window, (gpointer*)&widget);

      if (toplevel)
         gtkparasite_widget_tree_scan(parasite->widget_tree, toplevel);

      if (widget)
         gtkparasite_widget_tree_select_widget(parasite->widget_tree, widget);
   }
}


static void
on_highlight_widget(GtkWidget *grab_window, GdkEventMotion *event,
                    ParasiteWindow *parasite)
{
   GdkWindow *selected_window;
   gint x, y, width, height;

   if (parasite->highlight_window == NULL)
   {
      GdkColor color;

      color.red = 0;
      color.green = 0;
      color.blue = 65535;

      parasite->highlight_window = gtk_window_new(GTK_WINDOW_POPUP);
      gtk_widget_modify_bg(parasite->highlight_window, GTK_STATE_NORMAL,
                           &color);

      if (gtk_widget_is_composited(parasite->window))
      {
         gtk_window_set_opacity(GTK_WINDOW(parasite->highlight_window), 0.2);
      }
      else
      {
         /*
          * TODO: Do something different when there's no compositing manager.
          *       Draw a border or something.
          */
      }
   }

   gtk_widget_hide(parasite->highlight_window);

   selected_window = gdk_display_get_window_at_pointer(
      gtk_widget_get_display(grab_window), NULL, NULL);
   parasite->selected_window = selected_window;

   if (selected_window == NULL)
   {
      /* This window isn't in-process. Ignore it. */
      return;
   }

   gdk_window_get_origin(selected_window, &x, &y);
   gdk_drawable_get_size(GDK_DRAWABLE(selected_window), &width, &height);
   gtk_window_move(GTK_WINDOW(parasite->highlight_window), x, y);
   gtk_window_resize(GTK_WINDOW(parasite->highlight_window), width, height);
   gtk_widget_show(parasite->highlight_window);
}


static void
on_inspect_button_release(GtkWidget *button, GdkEventButton *event,
                          ParasiteWindow *parasite)
{
   GdkCursor *cursor;
   GdkEventMask events;

   events = GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
            GDK_POINTER_MOTION_MASK;

   if (parasite->grab_window == NULL)
   {
      parasite->grab_window = gtk_window_new(GTK_WINDOW_POPUP);
      gtk_widget_show(parasite->grab_window);
      gtk_window_resize(GTK_WINDOW(parasite->grab_window), 1, 1);
      gtk_window_move(GTK_WINDOW(parasite->grab_window), -100, -100);
      gtk_widget_add_events(parasite->grab_window, events);

      g_signal_connect(G_OBJECT(parasite->grab_window), "button_release_event",
                       G_CALLBACK(on_inspect_widget), parasite);
      g_signal_connect(G_OBJECT(parasite->grab_window), "motion_notify_event",
                       G_CALLBACK(on_highlight_widget), parasite);
   }

   cursor = gdk_cursor_new_for_display(gtk_widget_get_display(button),
                                       GDK_CROSSHAIR);
   gdk_pointer_grab(parasite->grab_window->window, FALSE,
                    events,
                    NULL,
                    cursor,
                    event->time);
   gdk_cursor_unref(cursor);
}


GtkWidget *
gtkparasite_inspect_button_new(ParasiteWindow *parasite)
{
   GtkWidget *button;

   button = gtk_button_new_with_label("Inspect");
   g_signal_connect(G_OBJECT(button), "button_release_event",
                    G_CALLBACK(on_inspect_button_release), parasite);

   return button;
}
