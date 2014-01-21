/* Calf Library
 * Jack Connector Window
 *
 * Copyright (C) 2014 Markus Schmidt / Krzysztof Foltman
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <calf/giface.h>
#include <calf/connector.h>
#include <calf/custom_ctl.h>

#include <jack/jack.h>
#include <jack/midiport.h>

using namespace std;
using namespace calf_plugins;

calf_connector::calf_connector(plugin_ctl_iface *plugin_, GtkWidget *toggle_)
{
    plugin = plugin_;
    toggle = toggle_;
    window = NULL;
    selector = NULL;
    create_window();
}

calf_connector::~calf_connector()
{
    
}

void calf_connector::on_destroy_window(GtkWidget *window, gpointer self)
{
    calf_connector *con = (calf_connector *)self;
    if (con->toggle)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(con->toggle), FALSE);
    //delete (calf_connector *)self;
}

void calf_connector::create_window()
{
    GtkRequisition req;
    char buf[256];
    
    // CREATE WINDOW
    
    string title = "Calf " + (string)plugin->get_metadata_iface()->get_label() + " Connector";
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), title.c_str());
    gtk_window_set_destroy_with_parent(GTK_WINDOW(window), TRUE);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    gtk_window_set_icon_name(GTK_WINDOW(window), "calf_plugin");
    gtk_window_set_role(GTK_WINDOW(window), "calf_connector");
    gtk_window_set_default_size(GTK_WINDOW(window), -1, -1);
    gtk_widget_set_name(GTK_WIDGET(window), "Calf-Connector");
    
    g_signal_connect(G_OBJECT(window), "destroy",
                     G_CALLBACK (on_destroy_window), (gpointer)this);
    
    // CONTAINER STUFF
    
    GtkHBox *hbox = GTK_HBOX(gtk_hbox_new(false, 8));
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(hbox));
    gtk_container_set_border_width (GTK_CONTAINER(window), 8);
    gtk_widget_set_name(GTK_WIDGET(hbox), "Calf-Container");
    
    GtkVBox *ports = GTK_VBOX(gtk_vbox_new(false, 0));
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(ports), false, false, 0);
    
    GtkWidget *inframe = calf_frame_new("Inputs");
    gtk_box_pack_start(GTK_BOX(ports), GTK_WIDGET(inframe), false, true, 10);
    gtk_container_set_border_width (GTK_CONTAINER(inframe), 8);
    
    GtkWidget *inputs = gtk_vbox_new(false, 5);
    gtk_container_add(GTK_CONTAINER(inframe), GTK_WIDGET(inputs));
    
    GtkWidget *outframe = calf_frame_new("Outputs");
    gtk_box_pack_start(GTK_BOX(ports), GTK_WIDGET(outframe), false, true, 10);
    gtk_container_set_border_width (GTK_CONTAINER(outframe), 8);
    
    GtkWidget *outputs = gtk_vbox_new(false, 5);
    gtk_container_add(GTK_CONTAINER(outframe), GTK_WIDGET(outputs));
    
    GtkWidget *midframe = calf_frame_new("MIDI");
    gtk_box_pack_start(GTK_BOX(ports), GTK_WIDGET(midframe), false, true, 10);
    gtk_container_set_border_width (GTK_CONTAINER(midframe), 8);
    
    GtkWidget *midi = gtk_vbox_new(false, 5);
    gtk_container_add(GTK_CONTAINER(midframe), GTK_WIDGET(midi));
    
    gtk_widget_show_all(window);
    
    // ALL THOSE PORT BUTTONS
    GtkRadioButton *last = NULL;
    int c = 0;
    for (int i = 0; i < plugin->get_metadata_iface()->get_input_count(); i++) {
        sprintf(buf, "Input #%d", (i + 1));
        GtkWidget *in = calf_radio_button_new(buf);
        gtk_box_pack_start(GTK_BOX(inputs), in, false, true, 0);
        gtk_widget_set_size_request(GTK_WIDGET(in), 110, 27);
        gtk_widget_show(GTK_WIDGET(in));
        if (!i)
            last = GTK_RADIO_BUTTON(in);
        else
            gtk_radio_button_set_group(GTK_RADIO_BUTTON(in), gtk_radio_button_get_group(last));
        c++;
    }
    if (!c)
        gtk_widget_hide(GTK_WIDGET(inframe));
    c = 0;
    for (int i = 0; i < plugin->get_metadata_iface()->get_output_count(); i++) {
        sprintf(buf, "Output #%d", (i + 1));
        GtkWidget *out = calf_radio_button_new(buf);
        gtk_box_pack_start(GTK_BOX(outputs), out, false, true, 0);
        gtk_widget_set_size_request(GTK_WIDGET(out), 110, 27);
        gtk_widget_show(GTK_WIDGET(out));
        if (!i and !last)
            last = GTK_RADIO_BUTTON(out);
        else
            gtk_radio_button_set_group(GTK_RADIO_BUTTON(out), gtk_radio_button_get_group(last));
        c++;
    }
    if (!c)
        gtk_widget_hide(GTK_WIDGET(outframe));
    c = 0;
    if (plugin->get_metadata_iface()->get_midi()) {
        GtkWidget *mid = calf_radio_button_new("MIDI");
        gtk_box_pack_start(GTK_BOX(midi), mid, false, true, 0);
        gtk_widget_set_size_request(GTK_WIDGET(mid), 110, 27);
        gtk_radio_button_set_group(GTK_RADIO_BUTTON(mid), gtk_radio_button_get_group(last));
        gtk_widget_show(GTK_WIDGET(mid));
        c++;
    }
    if (!c)
        gtk_widget_hide(GTK_WIDGET(midframe));
    
    selector = gtk_radio_button_get_group(last);
    
    // LIST STUFF
    
    GtkWidget *scroller = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(hbox), scroller, true, true, 0);
    gtk_widget_set_size_request(GTK_WIDGET(scroller), 240, 360);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_show(GTK_WIDGET(scroller));
    
    jacklist = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);
    GtkWidget *jackview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(jacklist));
    gtk_container_add(GTK_CONTAINER(scroller), jackview);
    gtk_widget_show(GTK_WIDGET(jackview));
    
    fill_list();
}

void calf_connector::fill_list()
{
    gtk_list_store_clear(jacklist);
    
}

void calf_connector::close()
{
    if (window)
        gtk_widget_destroy(GTK_WIDGET(window));
    //delete (calf_connector *)this;
}