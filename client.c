#include <gtk/gtk.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "messenger.h"
#include "list.h"

#define MAX_ARG_COUNT 10

int sockfd;
GThread *thread;
char my_login[LOGIN_LENGTH+1] = {0};
char server_ip[50] = {0};
unsigned char chat_type = TYPE_SOURCE_USER;
char chat_name[LOGIN_LENGTH+1] = {0};

GtkTextBuffer *buffer;
GtkWidget *label;
GtkWidget *edit_message;
GtkWidget *contents;
GtkWidget *window;
GtkTextIter iter_end;

int parsim(const char *text, char *arg[]);//Разбит text на аргументы arg
void *read_server(void *arg); //функция отдельного потока для чтения с сервера
void text_buffer_message_append(char* name, char *text, char* style); //непосредственное заполнение textview
int text_analyz(const char* text); //Проверка текста на наличие '/' и выполнение команд, если надо
void send_message(); //размещает сообщение в textview и отправляет его на серв 
void button_clicked(GtkWidget *button, gpointer data){send_message();}
void edit_message_activate(GtkWidget *window, gpointer data){send_message();}
void closeApp(GtkWidget *window, gpointer data){close(sockfd);gtk_main_quit();}
void create_interface();

//Работа с сервером
int client_login(char* login);

int main(int argc, char* argv[]) {
	int opt;
    while ((opt = getopt(argc, argv, ":s:l:")) != -1) {
        switch(opt) {
            case 's':
                strcpy(server_ip, optarg);
                break;
            case 'l':
                strcpy(my_login, optarg);
                break;
            default:
                break;
        }
    }
    
    if (my_login[0] == 0) {
    	printf("Where is your name? Go away!\n");
    	exit(1);
    }
    if (server_ip[0] == 0) strcpy(server_ip,"127.0.0.1");

	int len;
	struct sockaddr_in address;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
    {
        perror("socket");
        exit(1);
    }

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(server_ip);
	address.sin_port = htons(9734);
	len = sizeof(address);

	int result = connect(sockfd, (struct sockaddr *)&address, len);
	if (result < 0) {
		perror("Connection error ");
		exit(1);
	}

	if (client_login(my_login) < 0){
		printf("Can`t login. Try another name.\n");
		exit(1);
	}

	gdk_threads_init ();
	gtk_init(NULL, NULL);
    create_interface();
    g_thread_new( "read_server", read_server , NULL);
	gtk_main();
	gdk_threads_leave();
}
int client_login(char* login)
{
	unsigned char a; 
	if (write(sockfd, my_login, strlen(my_login)+1) <= 0){
		return -1;
	}
	if (read(sockfd, &a, sizeof(a)) <= 0 ){
		return -1;
	}
	if (a == TYPE_ANSWER_FAIL){
		return -1;
	}
	return 1;
}
void *read_server(void *arg) 
{
	int i,count;
	char sender[2*LOGIN_LENGTH+3] = {0};
	while (1){
		message_from mf;
		count = client_read_message(sockfd, &mf);
		if (count <= 0){
			break;
		}

		if (mf.sender.source_type == TYPE_SOURCE_GROUP){
			strcpy(sender,mf.sender.group_name);
			strcat(sender, " (");
			strcat(sender, mf.sender.login);
			strcat(sender, ")");
		}else{
			strcpy(sender, mf.sender.login);
		}
		gdk_threads_enter();
		text_buffer_message_append(sender, mf.message, "sender");
		gdk_threads_leave();
	}
	printf("Sorry, we lost server :( \n");
	close(sockfd);
	exit(0);
}
void text_buffer_message_append(char* name, char *text, char* style){
	gtk_text_buffer_insert_with_tags_by_name(buffer,&iter_end, name, -1, style, NULL);
	gtk_text_buffer_insert_with_tags_by_name(buffer,&iter_end, ": ", -1, style, NULL);

	gtk_text_buffer_insert (buffer,&iter_end, text, -1);
	gtk_text_buffer_insert (buffer,&iter_end, "\n", -1);
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW (contents), &iter_end, 0, 0, 0, 0);
}
int text_analyz(const char* text){
	if (text[0] == '/') {

		char arg[MAX_ARG_COUNT][MESSAGE_TEXT_LENGTH];
		char *argp[MAX_ARG_COUNT];
		int arg_count,c;
	    for(c=0; c< MAX_ARG_COUNT; c++) argp[c] = arg[c];
		arg_count = parsim(text+1,  argp);

		if(!strcmp(argp[0],"help")){
			gtk_text_buffer_insert_with_tags_by_name(buffer,&iter_end, 
						"\n#################################\n"
						"Вступить в группу: \n/goto *ИмяГруппы*\n\n"
						"Перейти к чату группы:\n/g *ИмяГруппы*\n\n"
						"Перейти к диалогу с пользователем:\n/u *ИмяПользователя*\n"
						"#################################\n\n"
						, -1, "italic", NULL);
			gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW (contents), &iter_end, 0, 0, 0, 0);
			return -1;
		}

		if (arg_count < 2) return -1;

		if (!strcmp(argp[0], "g")){
			char buf[40+LOGIN_LENGTH] = {0};
			chat_type = TYPE_SOURCE_GROUP;
			strcpy(chat_name, argp[1]);
			strcpy(buf, "Write to: ");
			strcat(buf, chat_name);
			strcat(buf, " (GROUP)");
			gtk_label_set_text(GTK_LABEL(label), buf);
		}else if (!strcmp(argp[0], "u")){
			char buf[40+LOGIN_LENGTH] = {0};
			chat_type = TYPE_SOURCE_USER;
			strcpy(chat_name, argp[1]);
			strcpy(buf, "Write to: ");
			strcat(buf, chat_name);
			gtk_label_set_text(GTK_LABEL(label), buf);
		}else if (!strcmp(argp[0], "goto")){
			unsigned char k = TYPE_QUERY_GO_TO_GROUP;
			
			if (write(sockfd, &k, sizeof(k)) <= 0) {
				perror("Cant send join_group");
				exit(1);
			}

			if (write(sockfd, argp[1], strlen(argp[1])+1) <= 0) {
				perror("Cant send join_group");
				exit(1);
			}
		}

		return -1;
	}
	return 1;
}
void send_message(){
	const char *text = gtk_entry_get_text(GTK_ENTRY((GtkWidget *) edit_message));
	if (!strcmp(text, "")) return;

	const char *t = text;
	if (text_analyz(t) < 0){
		gtk_entry_set_text(GTK_ENTRY((GtkWidget *) edit_message), "");
		return;
	}

	//Create message
	message_to m;
	m.recipient.destination_type = chat_type;
	strcpy(m.recipient.name, chat_name);
	strcpy(m.message, text);
	
	text_buffer_message_append(my_login, m.message, "my_login");
	gtk_entry_set_text(GTK_ENTRY((GtkWidget *) edit_message), "");
	gtk_widget_grab_focus(edit_message);

	//SEND MESSAGE TO SERVER!
	int count = client_send_message(sockfd, &m);
	if (count <= 0){
		printf("Can`t write message! :(\n");
		exit(1);
	}
}
void create_interface()
{
	GtkWidget *vbox;
	GtkWidget *hbox;

	GtkWidget *button;
	GtkWidget *sw;
	GtkWidget *ok_button;

	
	gdk_threads_enter();

	//WINDOW
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	char buf[LOGIN_LENGTH+100] = "Messenger v1.0   -   ";
	strcat(buf, my_login);

	gtk_window_set_title (GTK_WINDOW (window), buf);
	gtk_window_set_default_size(GTK_WINDOW(window), 450, 500);

	//SCROLL WINDOW
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_IN);

	//TEXTVIEW
	contents = gtk_text_view_new ();
	gtk_text_view_set_editable (GTK_TEXT_VIEW (contents), FALSE);
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (contents	));
	gtk_text_view_set_buffer (GTK_TEXT_VIEW (contents), buffer);
	gtk_container_add (GTK_CONTAINER (sw), contents);
	gtk_text_buffer_get_end_iter(buffer, &iter_end);

	//CREATE TAGS
	gtk_text_buffer_create_tag (buffer, "my_login",
	            "foreground", "red", "weight", PANGO_WEIGHT_BOLD, NULL);   
	gtk_text_buffer_create_tag (buffer, "sender",
	            "foreground", "blue", "weight", PANGO_WEIGHT_BOLD, NULL);  
	gtk_text_buffer_create_tag (buffer, "italic",
    	        "style", PANGO_STYLE_ITALIC, NULL); 

	//WRITE HELP MESSAGE
	gtk_text_buffer_insert_with_tags_by_name(buffer,&iter_end, 
		"Напишите /help ,чтобы просмотреть команды.\n\n"
		, -1, "italic", NULL);

	//EDIT MESSAGE
	edit_message = gtk_entry_new_with_max_length(MESSAGE_TEXT_LENGTH-1);
	g_signal_connect(GTK_OBJECT(edit_message), "activate", GTK_SIGNAL_FUNC(edit_message_activate), NULL);

	//BUTTON
	ok_button = gtk_button_new_with_label("Ok");
	g_signal_connect(GTK_OBJECT(ok_button), "clicked", GTK_SIGNAL_FUNC(button_clicked), NULL);

	//LABEL
	strcpy(chat_name, my_login);
	strcpy(buf, "Write to: ");strcat(buf, chat_name);
	label = gtk_label_new(buf);

	//BOXES
	vbox = gtk_vbox_new(FALSE, 0);
	hbox = gtk_hbox_new(FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 1);
	gtk_box_pack_start(GTK_BOX(hbox), edit_message, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), ok_button, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 1);

	gtk_container_add(GTK_CONTAINER(window), vbox);

	gtk_widget_grab_focus(edit_message);

	g_signal_connect(GTK_OBJECT(window), "destroy", GTK_SIGNAL_FUNC(closeApp), NULL);
	gtk_widget_show_all (window);
}

int parsim(const char *text, char *arg[])
{
    char c;
    char buf[MESSAGE_TEXT_LENGTH];
    char *cp = buf;
    int arg_count = 0;
    while (arg_count < MAX_ARG_COUNT){
        c = *(text++);
        if (c == '\0' || c == ' ' || c == '\t') {
            *cp = '\0';
            strcpy(arg[arg_count++], buf);
            while (c == ' ' || c == '\t') c = *(text++);
            if (c == '\0') break;
            cp = buf;
        }
        *(cp++) = c;
    }
    return arg_count;
}
