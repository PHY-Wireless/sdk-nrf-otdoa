/* blink speeds */
typedef enum {
	NONE,
	SLOW,
	MED,
	FAST,
	FLASH,
} BLINK_SPEED;

int init_led(void);
int init_button();
int set_led(int);
int toggle_led(void);
void set_blink_mode(BLINK_SPEED);
void set_blink_sleep(void);
void set_blink_prs(void);
void set_blink_download(void);
void set_blink_error(void);
