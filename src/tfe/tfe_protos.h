/* tfe.c */
struct snapshot_s;
extern void tfe_init(void);
extern int tfe_resources_init(void);
extern int tfe_cmdline_options_init(void);
extern int update_tfe_interface(void *v, void *param);
void get_disabled_state(int * param);
extern void tfe_reset(void);
extern void tfe_shutdown(void);
extern BYTE REGPARM1 tfe_read(WORD addr);
extern void REGPARM2 tfe_store(WORD addr, BYTE byte);
extern int tfe_read_snapshot_module(struct snapshot_s *s);
extern int tfe_write_snapshot_module(struct snapshot_s *s);
extern BYTE __stdcall TfeIo (WORD programcounter, BYTE address, BYTE write, BYTE value, ULONG nCycles);
extern int tfe_enumadapter_open(void);
extern int tfe_enumadapter(char **ppname, char **ppdescription);
extern int tfe_enumadapter_close(void);

/* tfearch.c */
#ifdef HAVE_TFE 
#else
  #error TFEARCH.H should not be included if HAVE_TFE is not defined!
#endif /* #ifdef HAVE_TFE */

extern int  tfe_arch_init(void);
extern void tfe_arch_pre_reset(void);
extern void tfe_arch_post_reset(void);
extern int  tfe_arch_activate(const char *interface_name);
extern void tfe_arch_deactivate(void);
extern void tfe_arch_set_mac(const BYTE mac[6]);
extern void tfe_arch_set_hashfilter(const DWORD hash_mask[2]);
extern void tfe_arch_recv_ctl( int bBroadcast,   /* broadcast */
                        int bIA,          /* individual address (IA) */
                        int bMulticast,   /* multicast if address passes the hash filter */
                        int bCorrect,     /* accept correct frames */
                        int bPromiscuous, /* promiscuous mode */
                        int bIAHash       /* accept if IA passes the hash filter */
                      );

extern void tfe_arch_line_ctl(int bEnableTransmitter, int bEnableReceiver);

extern void tfe_arch_transmit(int force,       /* FORCE: Delete waiting frames in transmit buffer */
                       int onecoll,     /* ONECOLL: Terminate after just one collision */
                       int inhibit_crc, /* INHIBITCRC: Do not append CRC to the transmission */
                       int tx_pad_dis,  /* TXPADDIS: Disable padding to 60 Bytes */
                       int txlength,    /* Frame length */
                       BYTE *txframe    /* Pointer to the frame to be transmitted */
                      );

extern int tfe_arch_receive(BYTE *pbuffer  ,    /* where to store a frame */
                     int  *plen,         /* IN: maximum length of frame to copy; 
                                            OUT: length of received frame 
                                            OUT can be bigger than IN if received frame was
                                                longer than supplied buffer */
                     int  *phashed,      /* set if the dest. address is accepted by the hash filter */
                     int  *phash_index,  /* hash table index if hashed == TRUE */   
                     int  *prx_ok,       /* set if good CRC and valid length */
                     int  *pcorrect_mac, /* set if dest. address is exactly our IA */
                     int  *pbroadcast,   /* set if dest. address is a broadcast address */
                     int  *pcrc_error    /* set if received frame had a CRC error */
                     );

extern int tfe_should_accept(unsigned char *buffer, int length, int *phashed, int *phash_index, 
                      int *pcorrect_mac, int *pbroadcast, int *pmulticast);

extern int tfe_arch_enumadapter_open(void);
extern int tfe_arch_enumadapter(char **ppname, char **ppdescription);
extern int tfe_arch_enumadapter_close(void);
extern char *tfe_getadapter(struct in_addr addr);

/* tfesupp.c */
extern void *lib_malloc(size_t size);
extern void *lib_realloc(void *p, size_t size);
extern void lib_free(void *ptr);
extern char *lib_stralloc(const char *str);
extern int util_string_set(char **str, const char *new_value);
extern unsigned long crc32_buf(const char *buffer, unsigned int len);

/* ulib.c */
extern TCHAR *uilib_select_file(HWND hwnd, const TCHAR *title, DWORD filterlist,
                                unsigned int type, int style);
extern TCHAR *uilib_select_file_autostart(HWND hwnd, const TCHAR *title,
                                          DWORD filterlist, unsigned int type,
                                          int style, int *autostart,
                                          char *resource_readonly);
extern void uilib_select_browse(HWND hwnd, const TCHAR *title,
                                DWORD filterlist, unsigned int type, int idc);
extern void uilib_show_options(HWND param);
extern void ui_show_text(HWND hParent, const char *szCaption,
                         const char *szHeader, const char *szText);
extern int ui_messagebox(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
extern void uilib_shutdown(void);
extern void uilib_dialogbox(uilib_dialogbox_param_t *param);
extern void uilib_get_general_window_extents(HWND hwnd, int *xsize, int *ysize);
extern void uilib_get_group_extent(HWND hwnd, uilib_dialog_group *group, int *xsize, int *ysize);
extern void uilib_move_and_adjust_group_width(HWND hwnd, uilib_dialog_group *group, int xpos);
extern void uilib_move_group(HWND hwnd, uilib_dialog_group *group, int xpos);
extern void uilib_adjust_group_width(HWND hwnd, uilib_dialog_group *group);
extern void uilib_move_and_adjust_element_width(HWND hwnd, int idc, int xpos);
extern void uilib_adjust_element_width(HWND hwnd, int idc);
extern void uilib_set_element_width(HWND hwnd, int idc, int xsize);
extern void uilib_localize_dialog(HWND hwnd, uilib_localize_dialog_param *param);

