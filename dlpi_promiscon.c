/* 

License : CDDL

NAME : dlpi_promiscona

cc -o dlpi_promiscon dlpi_promiscon.c -l dlpi

 */

#include <stdio.h>
#include <stdlib.h>
#include <libdlpi.h>
#include <strings.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/varargs.h>
#include <unistd.h>
#include <stropts.h>
#include <sys/ethernet.h>
#include <sys/zone.h>
#include <sys/byteorder.h>
#include <limits.h>
#include <inet/ip.h>
#include <inet/ip6.h>
#include <net/trill.h>

static int snaplen;

typedef struct dlpi_walk_arg {
  char	dwa_linkname[MAXLINKNAMELEN];
  int	dwa_type;	/* preference type above */
  int	dwa_s4;		/* IPv4 socket */
  int	dwa_s6;		/* IPv6 socket */
} dlpi_walk_arg_t;

static int strioctl(int, int, int, int, void *);
			       
			       
/*
 * Print an error.
 * Works like printf (fmt string and variable args)
 * except that it will substitute an error message
 * for a "%m" string (like syslog) and it calls
 * long_jump - it doesn't return to where it was
 * called from - it goes to the last setjmp().
 */
/* VARARGS1 */
void
pr_err(const char *fmt, ...)
{
  va_list ap;
  char buf[1024], *p2;
  const char *p1;

  (void) strcpy(buf, "dlpi_promiscon: ");
  p2 = buf + strlen(buf);

  /*
   * Note that we terminate the buffer with '\n' and '\0'.
   */
  for (p1 = fmt; *p1 != '\0' && p2 < buf + sizeof (buf) - 2; p1++) {
    if (*p1 == '%' && *(p1+1) == 'm') {
      const char *errstr;

      if ((errstr = strerror(errno)) != NULL) {
	*p2 = '\0';
	(void) strlcat(buf, errstr, sizeof (buf));
	p2 += strlen(p2);
      }
      p1++;
    } else {
      *p2++ = *p1;
    }
  }
  if (p2 > buf && *(p2-1) != '\n')
    *p2++ = '\n';
  *p2 = '\0';

  va_start(ap, fmt);
  /* LINTED: E_SEC_PRINTF_VAR_FMT */
  (void) vfprintf(stderr, buf, ap);
  va_end(ap);
}

/*
 * Store a copy of linkname associated with the DLPI handle.
 * Save errno before closing the dlpi handle so that the
 * correct error value is used if 'err' is a system error.
 */
void
pr_errdlpi(dlpi_handle_t dh, const char *cmd, int err)
{
  int save_errno = errno;
  char linkname[DLPI_LINKNAME_MAX];

  (void) strlcpy(linkname, dlpi_linkname(dh), sizeof (linkname));

  dlpi_close(dh);
  errno = save_errno;

  fprintf(stderr,"%s on \"%s\": %s", cmd, linkname, dlpi_strerror(err));
}


/*
 * Open `linkname' in raw/passive mode (see dlpi_open(3DLPI)). 
 * Also gather some information about the datalink useful for 
 * building the proper packet filters.
 */
int open_datalink(dlpi_handle_t *dhp, const char *linkname)
{
  int retval;
  int flags = DLPI_PASSIVE | DLPI_RAW;
  dlpi_walk_arg_t dwa;
  dlpi_info_t dlinfo;

  if ((retval = dlpi_open(linkname, dhp, flags)) != DLPI_SUCCESS) {
    pr_err("cannot open \"%s\": %s", linkname,
	   dlpi_strerror(retval));
    return 0;
  }

  if ((retval = dlpi_info(*dhp, &dlinfo, 0)) != DLPI_SUCCESS)
    pr_errdlpi(*dhp, "dlpi_info failed", retval);

  return 1;
}

/*
 * Initialize `dh' for packet capture using the provided arguments.
 */
void init_datalink(dlpi_handle_t dh, ulong_t snaplen, ulong_t chunksize) {
  int 	retv;
  int 	netfd;

  retv = dlpi_bind(dh, DLPI_ANY_SAP, NULL);
  if (retv != DLPI_SUCCESS)
    pr_errdlpi(dh, "cannot bind on", retv);

  (void) fprintf(stderr, "Using device %s ", dlpi_linkname(dh));

  /*
   * If Pflg not set - use physical level
   * promiscuous mode.  Otherwise - just SAP level.
   */
  (void) fprintf(stderr, "(promiscuous mode)\n");
  retv = dlpi_promiscon(dh, DL_PROMISC_PHYS);
  if (retv != DLPI_SUCCESS) {
    pr_errdlpi(dh, "promiscuous mode(physical) failed",
	       retv);
  }

  retv = dlpi_promiscon(dh, DL_PROMISC_SAP);
  if (retv != DLPI_SUCCESS)
    pr_errdlpi(dh, "promiscuous mode(SAP) failed", retv);
}

void usage(){
  printf("dlpi_promiscon [-i network interface name:default is net0] \n"
	 "Explain: This program makes network interface do promiscuous mode\n");
}

int main(int argc,char **argv){
  dlpi_handle_t dh;
  char *datalink = "net0";
  const int chunksize = 8 * 8192;

  int c=0;
  extern char *optarg;
  extern int optind, opterr;

  while((c = getopt(argc, argv, "hi:"))!=EOF){
    switch(c){
    case 'i':
      datalink = optarg;
      break;
    case 'h':
    default:
      usage();
      exit(1);
      break;
    }
  }

  argc -= optind;
  argv += optind;
  
  if(! open_datalink(&dh, datalink)){
    exit(1);
  }
  init_datalink(dh, snaplen, chunksize);
  while (1){
    sleep(1);
  }

}

