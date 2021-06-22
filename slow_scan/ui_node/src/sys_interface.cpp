#include <ros/ros.h>
#include <gtk/gtk.h>

#include "ui_node/global_param.h"

//functions
static void activate(GtkApplication *app , gpointer data);
void CSS_enable (const gchar *css_file);

//widgets
time_t panel_time_start;
GtkWidget *clock_real;
GtkWidget *clock_relative;
GtkWidget *is_connected_signal;
GtkWidget *system_status_signal;




int main(int argc,char*argv[])
{    
    time(&panel_time_start);
    ros::init(argc, argv, "UI");
    ros::NodeHandle n;

    GtkApplication *app;
    int app_status;

    app = gtk_application_new("org.rain.example" , G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app , "activate" , G_CALLBACK(activate) , NULL);
    app_status = g_application_run(G_APPLICATION(app) , argc , argv);

    g_object_unref(app);
    return app_status;
}



void CSS_enable(const gchar *css_file )
{
    GtkCssProvider *provider = gtk_css_provider_new ();
    GdkDisplay     *display = gdk_display_get_default ();
    GdkScreen      *screen = gdk_display_get_default_screen (display);
    
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER (provider),    GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_css_provider_load_from_file( GTK_CSS_PROVIDER(provider), g_file_new_for_path(css_file), NULL );
}



static gboolean time_handler()
{
    //refresh the real world time clock. 
    time_t curtime;
    time(&curtime);
    struct tm *loctime = localtime(&curtime);
    gchar *time_real = g_strdup_printf("%04d-%02d-%02d\n%02d:%02d:%02d",(1900+loctime->tm_year),(1+loctime->tm_mon),(loctime->tm_mday),(loctime->tm_hour), (loctime->tm_min), (loctime->tm_sec));
    gtk_label_set_label(GTK_LABEL(clock_real), time_real);
    
    //refresh the relative time which stands for how long the system keep running. 
    time_t panel_time = curtime - panel_time_start;
    struct tm *runtime = localtime(&panel_time);
    gchar *time_relative = g_strdup_printf("%02d days\n%02d:%02d:%02d", (runtime->tm_mday-1),(runtime->tm_hour-8), (runtime->tm_min), (runtime->tm_sec));
    gtk_label_set_label(GTK_LABEL(clock_relative), time_relative);

    //refresh the signal light to show if the client has connected to the server. 
    ros::param::get("s1_connect_sta", is_connected_sta);
    if(is_connected_sta == false)
        gtk_widget_set_name(is_connected_signal, "ics_Off");
    else 
        gtk_widget_set_name(is_connected_signal, "ics_On");

    //refresh the signal light to show if the system is running. 
    ros::param::get("s2_server_sta", local_server_sta);
    if(local_server_sta == false)
        gtk_widget_set_name(system_status_signal, "sss_Off");
    else 
        gtk_widget_set_name(system_status_signal, "sss_On");




  return TRUE;
}


static void activate(GtkApplication *app , gpointer data)
{
    //initialize the builder and XML UI settings, register a window and load the CSS config. 
    GtkBuilder *builder = gtk_builder_new();
    gtk_builder_add_from_file(builder , "/home/logank/catkin_ws/src/slow_scan/ui_node/ui/SSsystem.ui" , NULL);
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder , "SSs_window"));
    gtk_window_set_icon(GTK_WINDOW(window), gdk_pixbuf_new_from_file("/home/logank/catkin_ws/src/slow_scan/ui_node/SSs.ico", NULL));
    gtk_application_add_window(app , GTK_WINDOW(window));
    CSS_enable("/home/logank/catkin_ws/src/slow_scan/ui_node/ui/style.css");


    //get widgets according to ID in builder XML
    clock_real = GTK_WIDGET(gtk_builder_get_object(builder , "clock_real"));
    clock_relative = GTK_WIDGET(gtk_builder_get_object(builder , "clock_relative"));

    is_connected_signal = GTK_WIDGET(gtk_builder_get_object(builder , "is_connected_signal"));
    system_status_signal = GTK_WIDGET(gtk_builder_get_object(builder , "system_status_signal"));


    //set timer to have a timer interrupt. 
    g_timeout_add(100, (GSourceFunc) time_handler, NULL);

    //release the builder after using it. 
    g_object_unref(G_OBJECT(builder)); 
}