#include <sys/fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <slurm/spank.h>

SPANK_PLUGIN(stdio_fds, 1);

#define ENSURE_STDIN_VAR "SLURM_SPANK_ENSURE_STDIN"

int _check_fd_open(int fd) {
  if (fcntl(fd, F_GETFD) == -1) {
    if ( errno == EBADF ) {
      // fd is not open
      return 0;
    } else {
      /* got -1 but not EBADF
       * so something went wrong */
      return -1;
    }
  } else {
    /* Aha! We're open */
    return 1;
  }
}

int _ensure_stdin() {

    int rc, fd;

    /* Don't do anything unless we're srun */
    if (spank_context () != S_CTX_LOCAL)
      return 0;

    /* Check to see if stdin is open */
    rc = _check_fd_open(STDIN_FILENO);
    if(rc == -1) {
      slurm_error("spank: %s: Failed to check state of fd 0: %s", plugin_name, strerror(errno));
      goto fail;
    }
    else if ( rc == 0 ) {
      /* It's not open so open it */
      fd = open("/dev/null", 0);

      if ( fd == -1 ) {
        slurm_error("spank: %s: Failed to open /dev/null: %s", plugin_name, strerror(errno));
        goto fail;
      }
      else {
        slurm_verbose("spank: %s: open()'d /dev/null on fd %d", plugin_name, fd);
      }

      /* If we for some reason didn't get the file open'd on fd 0
       * then bomb. We could dup the fd explicitly but that feels wrong
       * since it should get fd 0 if it is not in use and if its in use 
       * then lets not touch it */
      if ( fd != 0 ) {
        slurm_error("spank: %s: expected to get fd %d, got fd %d instead",
                plugin_name, fd, STDIN_FILENO);
        close(fd);
        goto fail;
      }
    }

    return 0;

    fail:
      return -1;
}

int _str2bool(char *str) {

    if ( str != NULL && strlen(str) > 0 && str[0] == '1' )
      return 1;
    else
      return 0;
}

int slurm_spank_init (spank_t sp, int ac, char **av)
{

    char *val = NULL;
    int rc = 0;

    if ( _str2bool(getenv(ENSURE_STDIN_VAR)) ) {
      slurm_verbose("spank: %s: %s evaluated to TRUE, will ensure STDIN is open", 
                    plugin_name, ENSURE_STDIN_VAR);
      rc=_ensure_stdin();
    }

    return rc;
}
