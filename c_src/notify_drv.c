/* port_driver.c */

#include <stdio.h>
#include "erl_driver.h"
#include <libnotify/notify.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    ErlDrvPort port;
} notify_data;

typedef struct {
    char label[256];
    char msg[1024];
    unsigned int tm;
} notify_msg;

msg_error(notify_msg *nm) {
    nm->tm = 200;
    strcpy(nm->label, "ERROR!");
    strcpy(nm->msg, "You got an error.");
}


static unsigned int parse_int(char *buffer, unsigned int len) {
    unsigned int ret = 0;

    while(len > 1) {
	ret |= (unsigned int)*(buffer++);
	ret << 8;
	len--;
    }
    ret |= (unsigned int) *buffer;

    return ret;
}

/* 
 * -----------------------------------------------------------------------
 * | tm x 4 bytes | label_len x 1 byte | msg_len x 4 bytes | label | msg |
 * -----------------------------------------------------------------------
 */

static int parse_msg(notify_msg *nm, char *buffer, int len) {
    unsigned char l_len;
    unsigned int m_len;
    memset(nm, 0, sizeof(notify_msg));
    if (len < 9) {
	msg_error(nm);
   	return -1;
    }
    /* dbg: 
    fprintf(stderr, "tm: {%d, %d, %d, %d}\r\n",buffer[0], buffer[1], buffer[2], buffer[3]);
    fprintf(stderr, "lm: {%d, %d, %d, %d}\r\n",buffer[5], buffer[6], buffer[7], buffer[8]);
    */

    nm->tm = parse_int(buffer, 4);
    l_len  = parse_int(buffer + 4, 1);
    m_len  = parse_int(buffer + 5, 4);
    /* dbg: 
    fprintf(stderr, "tm %d, l_len %d, m_len %d\r\n", nm->tm, l_len, m_len);
    */

    if (len < (l_len + m_len + 9)) {
	msg_error(nm);
   	return -1;

    }
  
    memcpy(nm->label, buffer + 9, (size_t)l_len);
    nm->label[l_len] = '\0';

    if (m_len < 1023) {
	memcpy(nm->msg, buffer + 9 + l_len, (size_t)m_len);
	nm->msg[m_len] = '\0';
    } else {
	memcpy(nm->msg, buffer + 9 + l_len, 1022);
	nm->msg[1023] = '\0';
    }
    /* dbg: 
    fprintf(stderr, "parse got: tm %d, label %s, msg %s\r\n", nm->tm, nm->label, nm->msg);
    */

    return 1;
}


static ErlDrvData notify_drv_start(ErlDrvPort port, char *buff)
{
    notify_data* d = (notify_data*)driver_alloc(sizeof(notify_data));
    notify_init("Erlang");
    /* dbg: 
    fprintf(stderr, "notify_drv: start\r\n");
    */
    d->port = port;
    return (ErlDrvData)d;
}

static void notify_drv_stop(ErlDrvData handle)
{
    /* dbg: 
    fprintf(stderr, "notify_drv: stop\r\n");
    */
    driver_free((char*)handle);
}

static void notify_drv_output(ErlDrvData handle, char *buffer, int len) {
    NotifyNotification *n;
	NotifyNotification *example;
    notify_data* d = (notify_data*)handle;
    notify_msg nm;
    char res = 0;
	char name[40] = "Sample Notification";
	GError *error = NULL;

    parse_msg(&nm, buffer, len);

    /* dbg: 
    fprintf(stderr, "notify_drv: output %s\r\n", buffer);
    */

    n = notify_notification_new(nm.label, nm.msg, NULL, NULL);
    notify_notification_set_timeout(n, nm.tm);
    notify_notification_set_category(n, "erlang");
    notify_notification_set_urgency(n, NOTIFY_URGENCY_CRITICAL);
    //notify_notification_set_hint_int32(n, "value", 1024); 
    notify_notification_set_hint_string(n, "x-canonical-private-synchronous", "");
	
    if (!notify_notification_show (n, NULL)) {
	fprintf(stderr, "Error: failed to send notification\r\n");
	res = 1;
    }
    g_object_unref(G_OBJECT(n));
    driver_output(d->port, &res, 1);
}

ErlDrvEntry notify_driver_entry = {
    NULL,                       /* F_PTR init, N/A */
    notify_drv_start,          /* L_PTR start, called when port is opened */
    notify_drv_stop,           /* F_PTR stop, called when port is closed */
    notify_drv_output,         /* F_PTR output, called when erlang has sent */
    NULL,                       /* F_PTR ready_input, called when input descriptor ready */
    NULL,                       /* F_PTR ready_output, called when output descriptor ready */
    "notify_drv",              /* char *driver_name, the argument to open_port */
    NULL,                       /* F_PTR finish, called when unloaded */
    NULL,                       /* F_PTR control, port_command callback */
    NULL,                       /* F_PTR timeout, reserved */
    NULL                        /* F_PTR outputv, reserved */
};

DRIVER_INIT(notify_drv) /* must match name in driver_entry */
{
    return &notify_driver_entry;
}
