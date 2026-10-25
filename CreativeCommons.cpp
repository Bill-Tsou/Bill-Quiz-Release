/*
 * this dialog was built on 28/09/18 21:07
 * ported from Win32 GDI to GTK3 for macOS and Ubuntu
 * all of the words are using UTF-8 charset
*/

#include <iostream>
#include <gtk/gtk.h>
#ifdef __APPLE__
#include <unistd.h>
#include <CoreGraphics/CoreGraphics.h>
#include <objc/runtime.h>
#include <objc/message.h>
#endif

#include "CreativeCommons.h"
#include "Menu.h"

using namespace std;

extern int Programme_Language;

#ifdef __APPLE__
// check whether this programme still has any normal window shown on the screen
static bool HasOnScreenWindow()
{
	bool found = false;
	CFArrayRef list = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);
	if(list == NULL)
		return false;

	for(CFIndex i = 0; i < CFArrayGetCount(list) && !found; i++)
	{
		CFDictionaryRef info = (CFDictionaryRef)CFArrayGetValueAtIndex(list, i);
		CFNumberRef pid_ref = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowOwnerPID);
		CFNumberRef layer_ref = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowLayer);
		int pid = 0, layer = -1;

		if(pid_ref != NULL)
			CFNumberGetValue(pid_ref, kCFNumberIntType, &pid);
		if(layer_ref != NULL)
			CFNumberGetValue(layer_ref, kCFNumberIntType, &layer);
		if(pid == getpid() && layer == 0)
			found = true;
	}

	CFRelease(list);
	return found;
}

// call an Objective-C method without argument through the runtime C API
static id ObjcSend(id object, const char *selector)
{
	return ((id (*)(id, SEL))objc_msgSend)(object, sel_registerName(selector));
}

// the application which was in front before the dialog is shown, e.g. the terminal
static id PreviousFrontApp = NULL;

// a programme started from the terminal is not activated by macOS,
// bring it to the front so that the dialog is shown above the other windows,
// call it after the dialog is shown (the native window exists then)
static void ActivateProgramme()
{
	id workspace = ObjcSend((id)objc_getClass("NSWorkspace"), "sharedWorkspace");
	PreviousFrontApp = ObjcSend(ObjcSend(workspace, "frontmostApplication"), "retain");

	id app = ObjcSend((id)objc_getClass("NSApplication"), "sharedApplication");
	((void (*)(id, SEL, BOOL))objc_msgSend)(app, sel_registerName("activateIgnoringOtherApps:"), YES);

	//gtk_window_set_keep_above() does not change the window level on macOS,
	//put all windows of this programme (only the dialog) on the floating level
	const long floating_window_level = 3;	//NSFloatingWindowLevel
	id windows = ObjcSend(app, "windows");
	unsigned long count = ((unsigned long (*)(id, SEL))objc_msgSend)(windows, sel_registerName("count"));
	for(unsigned long i = 0; i < count; i++)
	{
		id nswindow = ((id (*)(id, SEL, unsigned long))objc_msgSend)(windows,
			sel_registerName("objectAtIndex:"), i);
		((void (*)(id, SEL, long))objc_msgSend)(nswindow, sel_registerName("setLevel:"), floating_window_level);
		ObjcSend(nswindow, "orderFrontRegardless");
	}
}

// give the focus back to the terminal after the dialog is closed
static void RestorePreviousFrontApp()
{
	if(PreviousFrontApp == NULL)
		return;

	const unsigned long activate_ignoring_other_apps = 1 << 1;	//NSApplicationActivateIgnoringOtherApps
	((BOOL (*)(id, SEL, unsigned long))objc_msgSend)(PreviousFrontApp,
		sel_registerName("activateWithOptions:"), activate_ignoring_other_apps);
	ObjcSend(PreviousFrontApp, "release");
	PreviousFrontApp = NULL;
}
#endif

// keep processing the GTK events until the closed dialog is really removed from the screen,
// the terminal programme does not run the GTK main loop anymore after the dialog returns
static void FinishClosingDialog()
{
#ifdef __APPLE__
	// macOS closes the window with an animation which only goes on while the events are processed
	gint64 deadline = g_get_monotonic_time() + G_USEC_PER_SEC;	//wait at most 1 second
	while(HasOnScreenWindow() && g_get_monotonic_time() < deadline)
	{
		while(gtk_events_pending())
			gtk_main_iteration_do(FALSE);
		g_usleep(10 * 1000);
	}
	RestorePreviousFrontApp();
#endif
	while(gtk_events_pending())
		gtk_main_iteration_do(FALSE);
	gdk_display_flush(gdk_display_get_default());
}

static const char *LicenseTextEn[] = {
	"BY (Attribution): You should tag the one who designed the programme when sharing.",
	"NC (Non-commercial): You are NOT ALLOWED to use this programme with commercial purposes.",
	"ND (No Derivative Works): You cannot share the programme when you've done some modifications."
};

static const char *LicenseTextCh[] = {
	"BY 姓名標示：當您分享此程式時，應當標註程式設計者姓名",
	"NC 非商業用途：這個程式不得出售、轉賣",
	"ND 禁止改作：您需要保持程式原本的功能，分享時不得任意修改本程式的用途"
};

static const char LicenseFooter[] = "Creative Commons, Taiwan";

static void OnButtonOK(GtkWidget *widget, gpointer window)
{	//button was pressed, close the dialog
	gtk_widget_destroy(GTK_WIDGET(window));
}

static void ShowCreativeCommonsText()
{	//show the license in the terminal when the dialog cannot be created
	const char **text = (Programme_Language == Chinese ? LicenseTextCh : LicenseTextEn);

	ChangeColour(COLOR_WHITE);
	cout << (Programme_Language == Chinese ? DialogTitleCh : DialogTitleEn) << endl << endl;
	for(int i = 0; i < 3; i++)
		cout << "  " << text[i] << endl;
	cout << endl << "  " << LicenseFooter << endl << endl;
	ChangeColour(COLOR_NORMAL);
}

void ShowCreativeCommonsDialog()
{
	static int gtk_ready = -1;	//initialize GTK only once
	if(gtk_ready == -1)
		gtk_ready = gtk_init_check(NULL, NULL) ? 1 : 0;

	if(gtk_ready == 0)
	{
		ShowCreativeCommonsText();
		return;
	}

	const char **text = (Programme_Language == Chinese ? LicenseTextCh : LicenseTextEn);

	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), (Programme_Language == Chinese ? DialogTitleCh : DialogTitleEn));
	gtk_window_set_default_size(GTK_WINDOW(window), 410, 510);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	//Change the font of the dialog
	GtkCssProvider *css = gtk_css_provider_new();
	char css_text[100];
	snprintf(css_text, sizeof(css_text), "* { font-family: Arial, sans-serif; font-size: %dpx; }", FontHeight - 10);
	gtk_css_provider_load_from_data(css, css_text, -1, NULL);
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(css),
		GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	g_object_unref(css);

	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, FontHeight / 2);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	//show the Creative Commons picture on the top of the dialog
	GtkWidget *image = gtk_image_new_from_file(PATH_CREATIVECOMMONS_IMAGE);
	gtk_box_pack_start(GTK_BOX(vbox), image, FALSE, FALSE, 0);

	//show the context according to the language user selected
	for(int i = 0; i < 3; i++)
	{
		GtkWidget *label = gtk_label_new(text[i]);
		gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
		gtk_label_set_max_width_chars(GTK_LABEL(label), 1);	//wrap with the window width
		gtk_label_set_xalign(GTK_LABEL(label), 0.0);
		gtk_widget_set_margin_start(label, 10);
		gtk_widget_set_margin_end(label, 10);
		gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	}

	GtkWidget *footer = gtk_label_new(LicenseFooter);
	gtk_box_pack_start(GTK_BOX(vbox), footer, FALSE, FALSE, 0);

	//Create the button
	GtkWidget *button = gtk_button_new_with_label("OK");
	gtk_widget_set_margin_start(button, 10);
	gtk_widget_set_margin_end(button, 10);
	gtk_widget_set_margin_bottom(button, 10);
	g_signal_connect(button, "clicked", G_CALLBACK(OnButtonOK), window);
	gtk_box_pack_end(GTK_BOX(vbox), button, FALSE, FALSE, 0);

	//show the dialog above the other windows
	gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
	gtk_widget_show_all(window);
#ifdef __APPLE__
	ActivateProgramme();
#endif
	gtk_window_present(GTK_WINDOW(window));

	//run the main loop until the dialog is closed
	gtk_main();

	FinishClosingDialog();
}
