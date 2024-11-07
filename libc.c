/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>
#include <errno.h>

int errno;

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

void perror(void) 
{
  /* 
  NO ME ESTA DEJANDO IMPORTAR #include <string.h>
  DE MOMENTO LO PONGO 'FEO'
  > Supongo que habria que implementarlo a mano

  // Como es un array --> En C se usar strcpy para assignar strings
  //                      char* strcpy(char* destination, const char* source);
  char msg[256];

  switch(errno) {
    case ENOSYS:
      strcpy(msg, "Function not implemented");
      break;
    default:
      strcpy(msg, "Error not described");
      break;
  }
  write(1, msg, strlen(msg));
  */

  switch(errno) {
    char* msg;
    case EPERM:
      msg = "Operation not permitted";
        write(1, msg, strlen(msg));
        break;
    case ENOENT:
      msg = "No such file or directory";
        write(1, msg, strlen(msg));
        break;
    case ESRCH:
      msg = "No such process";
        write(1, msg, strlen(msg));
        break;
    case EINTR:
      msg = "Interrupted system call";
        write(1, msg, strlen(msg));
        break;
    case EIO:
      msg = "I/O error";
        write(1, msg, strlen(msg));
        break;
    case ENXIO:
      msg = "No such device or address";
        write(1, msg, strlen(msg));
        break;
    case E2BIG:
      msg = "Arg list too long";
        write(1, msg, strlen(msg));
        break;
    case ENOEXEC:
      msg = "Exec format error";
        write(1, msg, strlen(msg));
        break;
    case EBADF:
      msg = "Bad file number";
        write(1, msg, strlen(msg));
        break;
    case ECHILD:
      msg = "No child processes";
        write(1, msg, strlen(msg));
        break;
    case EAGAIN:
      msg = "Try again";
        write(1, msg, strlen(msg));
        break;
    case ENOMEM:
      msg = "Out of memory";
        write(1, msg, strlen(msg));
        break;
    case EACCES:
      msg = "Permission denied";
        write(1, msg, strlen(msg));
        break;
    case EFAULT:
      msg = "Bad address";
        write(1, msg, strlen(msg));
        break;
    case ENOTBLK:
      msg = "Block device required";
        write(1, msg, strlen(msg));
        break;
    case EBUSY:
      msg = "Device or resource busy";
        write(1, msg, strlen(msg));
        break;
    case EEXIST:
      msg = "File exists";
        write(1, msg, strlen(msg));
        break;
    case EXDEV:
      msg = "Cross-device link";
        write(1, msg, strlen(msg));
        break;
    case ENODEV:
      msg = "No such device";
        write(1, msg, strlen(msg));
        break;
    case ENOTDIR:
      msg = "Not a directory";
        write(1, msg, strlen(msg));
        break;
    case EISDIR:
      msg = "Is a directory";
        write(1, msg, strlen(msg));
        break;
    case EINVAL:
      msg = "Invalid argument";
        write(1, msg, strlen(msg));
        break;
    case ENFILE:
      msg = "File table overflow";
        write(1, msg, strlen(msg));
        break;
    case EMFILE:
      msg = "Too many open files";
        write(1, msg, strlen(msg));
        break;
    case ENOTTY:
      msg = "Not a typewriter";
        write(1, msg, strlen(msg));
        break;
    case ETXTBSY:
      msg = "Text file busy";
        write(1, msg, strlen(msg));
        break;
    case EFBIG:
      msg = "File too large";
        write(1, msg, strlen(msg));
        break;
    case ENOSPC:
      msg = "No space left on device";
        write(1, msg, strlen(msg));
        break;
    case ESPIPE:
      msg = "Illegal seek";
        write(1, msg, strlen(msg));
        break;
    case EROFS:
      msg = "Read-only file system";
        write(1, msg, strlen(msg));
        break;
    case EMLINK:
      msg = "Too many links";
        write(1, msg, strlen(msg));
        break;
    case EPIPE:
      msg = "Broken pipe";
        write(1, msg, strlen(msg));
        break;
    case EDOM:
      msg = "Math argument out of domain of func";
        write(1, msg, strlen(msg));
        break;
    case ERANGE:
      msg = "Math result not representable";
        write(1, msg, strlen(msg));
        break;
    case EDEADLK:
      msg = "Resource deadlock would occur";
        write(1, msg, strlen(msg));
        break;
    case ENAMETOOLONG:
      msg = "File name too long";
        write(1, msg, strlen(msg));
        break;
    case ENOLCK:
      msg = "No record locks available";
        write(1, msg, strlen(msg));
        break;
    case ENOSYS:
      msg = "Function not implemented";
        write(1, msg, strlen(msg));
        break;
    case ENOTEMPTY:
      msg = "Directory not empty";
        write(1, msg, strlen(msg));
        break;
    case ELOOP:
      msg = "Too many symbolic links encountered";
        write(1, msg, strlen(msg));
        break;
    case EWOULDBLOCK:
      msg = "Operation would block";
        write(1, msg, strlen(msg));
        break;
    case ENOMSG:
      msg = "No message of desired type";
        write(1, msg, strlen(msg));
        break;
    case EIDRM:
      msg = "Identifier removed";
        write(1, msg, strlen(msg));
        break;
    case ECHRNG:
      msg = "Channel number out of range";
        write(1, msg, strlen(msg));
        break;
    case EL2NSYNC:
      msg = "Level 2 not synchronized";
        write(1, msg, strlen(msg));
        break;
    case EL3HLT:
      msg = "Level 3 halted";
        write(1, msg, strlen(msg));
        break;
    case EL3RST:
      msg = "Level 3 reset";
        write(1, msg, strlen(msg));
        break;
    case ELNRNG:
      msg = "Link number out of range";
        write(1, msg, strlen(msg));
        break;
    case EUNATCH:
      msg = "Protocol driver not attached";
        write(1, msg, strlen(msg));
        break;
    case ENOCSI:
      msg = "No CSI structure available";
        write(1, msg, strlen(msg));
        break;
    case EL2HLT:
      msg = "Level 2 halted";
        write(1, msg, strlen(msg));
        break;
    case EBADE:
      msg = "Invalid exchange";
        write(1, msg, strlen(msg));
        break;
    case EBADR:
      msg = "Invalid request descriptor";
        write(1, msg, strlen(msg));
        break;
    case EXFULL:
      msg = "Exchange full";
        write(1, msg, strlen(msg));
        break;
    case ENOANO:
      msg = "No anode";
        write(1, msg, strlen(msg));
        break;
    case EBADRQC:
      msg = "Invalid request code";
        write(1, msg, strlen(msg));
        break;
    case EBADSLT:
      msg = "Invalid slot";
        write(1, msg, strlen(msg));
        break;
    case EDEADLOCK:
      msg = "File locking deadlock error";
        write(1, msg, strlen(msg));
        break;
    case EBFONT:
      msg = "Bad font file format";
        write(1, msg, strlen(msg));
        break;
    case ENOSTR:
      msg = "Device not a stream";
        write(1, msg, strlen(msg));
        break;
    case ENODATA:
      msg = "No data available";
        write(1, msg, strlen(msg));
        break;
    case ETIME:
      msg = "Timer expired";
        write(1, msg, strlen(msg));
        break;
    case ENOSR:
      msg = "Out of streams resources";
        write(1, msg, strlen(msg));
        break;
    case ENONET:
      msg = "Machine is not on the network";
        write(1, msg, strlen(msg));
        break;
    case ENOPKG:
      msg = "Package not installed";
        write(1, msg, strlen(msg));
        break;
    case EREMOTE:
      msg = "Object is remote";
        write(1, msg, strlen(msg));
        break;
    case ENOLINK:
      msg = "Link has been severed";
        write(1, msg, strlen(msg));
        break;
    case EADV:
      msg = "Advertise error";
        write(1, msg, strlen(msg));
        break;
    case ESRMNT:
      msg = "Srmount error";
        write(1, msg, strlen(msg));
        break;
    case ECOMM:
      msg = "Communication error on send";
        write(1, msg, strlen(msg));
        break;
    case EPROTO:
      msg = "Protocol error";
        write(1, msg, strlen(msg));
        break;
    case EMULTIHOP:
      msg = "Multihop attempted";
        write(1, msg, strlen(msg));
        break;
    case EDOTDOT:
      msg = "RFS specific error";
        write(1, msg, strlen(msg));
        break;
    case EBADMSG:
      msg = "Not a data message";
        write(1, msg, strlen(msg));
        break;
    case EOVERFLOW:
      msg = "Value too large for defined data type";
        write(1, msg, strlen(msg));
        break;
    case ENOTUNIQ:
      msg = "Name not unique on network";
        write(1, msg, strlen(msg));
        break;
    case EBADFD:
      msg = "File descriptor in bad state";
        write(1, msg, strlen(msg));
        break;
    case EREMCHG:
      msg = "Remote address changed";
        write(1, msg, strlen(msg));
        break;
    case ELIBACC:
      msg = "Can not access a needed shared library";
        write(1, msg, strlen(msg));
        break;
    case ELIBBAD:
      msg = "Accessing a corrupted shared library";
        write(1, msg, strlen(msg));
        break;
    case ELIBSCN:
      msg = ".lib section in a.out corrupted";
        write(1, msg, strlen(msg));
        break;
    case ELIBMAX:
      msg = "Attempting to link in too many shared libraries";
        write(1, msg, strlen(msg));
        break;
    case ELIBEXEC:
      msg = "Cannot exec a shared library directly";
        write(1, msg, strlen(msg));
        break;
    case EILSEQ:
      msg = "Illegal byte sequence";
        write(1, msg, strlen(msg));
        break;
    case ERESTART:
      msg = "Interrupted system call should be restarted";
        write(1, msg, strlen(msg));
        break;
    case ESTRPIPE:
      msg = "Streams pipe error";
        write(1, msg, strlen(msg));
        break;
    case EUSERS:
      msg = "Too many users";
        write(1, msg, strlen(msg));
        break;
    case ENOTSOCK:
      msg = "Socket operation on non-socket";
        write(1, msg, strlen(msg));
        break;
    case EDESTADDRREQ:
      msg = "Destination address required";
        write(1, msg, strlen(msg));
        break;
    case EMSGSIZE:
      msg = "Message too long";
        write(1, msg, strlen(msg));
        break;
    case EPROTOTYPE:
      msg = "Protocol wrong type for socket";
        write(1, msg, strlen(msg));
        break;
    case ENOPROTOOPT:
      msg = "Protocol not available";
        write(1, msg, strlen(msg));
        break;
    case EPROTONOSUPPORT:
      msg = "Protocol not supported";
        write(1, msg, strlen(msg));
        break;
    case ESOCKTNOSUPPORT:
      msg = "Socket type not supported";
        write(1, msg, strlen(msg));
        break;
    case EOPNOTSUPP:
      msg = "Operation not supported on transport endpoint";
        write(1, msg, strlen(msg));
        break;
    case EPFNOSUPPORT:
      msg = "Protocol family not supported";
        write(1, msg, strlen(msg));
        break;
    case EAFNOSUPPORT:
      msg = "Address family not supported by protocol";
        write(1, msg, strlen(msg));
        break;
    case EADDRINUSE:
      msg = "Address already in use";
        write(1, msg, strlen(msg));
        break;
    case EADDRNOTAVAIL:
      msg = "Cannot assign requested address";
        write(1, msg, strlen(msg));
        break;
    case ENETDOWN:
      msg = "Network is down";
        write(1, msg, strlen(msg));
        break;
    case ENETUNREACH:
      msg = "Network is unreachable";
        write(1, msg, strlen(msg));
        break;
    case ENETRESET:
      msg = "Network dropped connection because of reset";
        write(1, msg, strlen(msg));
        break;
    case ECONNABORTED:
      msg = "Software caused connection abort";
        write(1, msg, strlen(msg));
        break;
    case ECONNRESET:
      msg = "Connection reset by peer";
        write(1, msg, strlen(msg));
        break;
    case ENOBUFS:
      msg = "No buffer space available";
        write(1, msg, strlen(msg));
        break;
    case EISCONN:
      msg = "Transport endpoint is already connected";
        write(1, msg, strlen(msg));
        break;
    case ENOTCONN:
      msg = "Transport endpoint is not connected";
        write(1, msg, strlen(msg));
        break;
    case ESHUTDOWN:
      msg = "Cannot send after transport endpoint shutdown";
        write(1, msg, strlen(msg));
        break;
    case ETOOMANYREFS:
      msg = "Too many references: cannot splice";
        write(1, msg, strlen(msg));
        break;
    case ETIMEDOUT:
      msg = "Connection timed out";
        write(1, msg, strlen(msg));
        break;
    case ECONNREFUSED:
      msg = "Connection refused";
        write(1, msg, strlen(msg));
        break;
    case EHOSTDOWN:
      msg = "Host is down";
        write(1, msg, strlen(msg));
        break;
    case EHOSTUNREACH:
      msg = "No route to host";
        write(1, msg, strlen(msg));
        break;
    case EALREADY:
      msg = "Operation already in progress";
        write(1, msg, strlen(msg));
        break;
    case EINPROGRESS:
      msg = "Operation now in progress";
        write(1, msg, strlen(msg));
        break;
    case ESTALE:
      msg = "Stale NFS file handle";
        write(1, msg, strlen(msg));
        break;
    case EUCLEAN:
      msg = "Structure needs cleaning";
        write(1, msg, strlen(msg));
        break;
    case ENOTNAM:
      msg = "Not a XENIX named type file";
        write(1, msg, strlen(msg));
        break;
    case ENAVAIL:
      msg = "No XENIX semaphores available";
        write(1, msg, strlen(msg));
        break;
    case EISNAM:
      msg = "Is a named type file";
        write(1, msg, strlen(msg));
        break;
    case EREMOTEIO:
      msg = "Remote I/O error";
        write(1, msg, strlen(msg));
        break;
    default:
      msg = "Error not described";
      write(1, msg, strlen(msg));
      break;
  }
}