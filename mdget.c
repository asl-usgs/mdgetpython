#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
extern int errno;
int dbg=0;
char *replace(char *, int, int);
main(int argc, char **argv)
{

    int bnfp, out;
    int filePhase;
    struct sockaddr_in sin;
    struct hostent *hp, *gethostbyname();
    int i,l,j,k,m,port = 2052;
    char chr, req[80], ans[10241], host[50] = "137.227.224.97"; // Address of public metadata Server in DMZ
    char dirstub[80], filename[80];
    char *pnt;
    int off,flen;
    int exitstatus=0;
    if(argc <= 1) {
    printf("MDGET version 1.3 2013/03/06 def server=%s port=%d\n",host,port);
        printf("   Normal cooked response command (-cooked is assumed and optional) : \n\n");
        printf("   -s regexp where regexp is a regular expression for a FDSN/SEED in NNSSSSSCCCLL\n");
        printf("   -a AGENCY.DEPLOY.STATION -coord is for IADSR coordinates where * can be used at the end on any portion\n");
        printf("      ADSL only contains coordinates, descriptions, etc, but no seismometer responses\n");
        printf("      A.D.S.L  The .L is optional.  An * can be at the end of any portion. Most useful 'FDSN.IR.*'\n");
        printf("-s regexp [-b date][-e date|-d dur[d]][-xml][-u um|nm]\n");
        printf("\n       This is the '-cooked' response mode.  Get all matching channel epochs\n");
        printf("       betweeen the given dates and return in SAC format (default) or XML format\n");
        printf("       in the displacement units specified (default is nm) for channels matching\n");
        printf("       the regular expression.  Note : -cooked is the default command.\n");
        printf("\n  Other possible commands :\n");
        printf("-alias [-s NNSSSSS|-a A.S.D.L]  return the aliases for the given station\n");
        printf("-orient -s NNSSSSS [-b date][-e date][-d dur[d]][-xml] return coord/orientation epochs\n");
        printf("-coord [-s NNSSSSS|-a A.S.D.L] [-b date][-e date][-d dur[d]][-xml][-kml] return coord/orientation epochs by channel\n");
        printf("-lsc -s regexp Return a list of channels matching the regular expression\n");
        printf("-desc [-s NNSSSSS|-a A.S.D.L]  return the NEIC long station name and operators \n");
        printf("-resp[:dirstub] regexp get all RESP files and put in dirstub directory\n");
        printf("-dataless[:dirstub] get all matching dataless seed volumes and put in dirstub\n");
        printf("-station -s NNSSSSS [-xml] return all information about a station optionally in XML (no wildcards!)\n");
        printf("-kml -s NNSSSSSSCCCLL KML file returned for all matching station\n:");
        printf("\n  Notes on various options :\n");
        printf("date     Dates can be of the form YYYY,DDD-hh:mm:ss or YYYY/MM/DD-hh:mm:ss\n");
        printf("         if -b all is used, then all epoch from 1970 to present will be returned\n");
        printf("         if -b and -e are omitted, they default to the current time\n");
        printf("         if -b is present and -e is omitted, the end date will equal the begin date\n");
        printf("regexp   To specify and exact channel match use the full NNSSSSSCCLL (use quotes!)\n");
        printf("         For pattern matching '.' matches any single character, [ABC] for A, B, or C\n");
        printf("         '.*' matches anything zero or more times e.g. US.* would match the US network\n");
        printf("            always enclose '.*' in quotes since most shells give '*' other meanings\n");
        printf("         [A-Z] allows any character A to Z inclusive in the position.\n");
        printf("         A|B means matches A or B so 'US.*|AT.*|IU.*' matches anything in the US, AT or IU nets\n");
        printf("         Examples : 'USDUG  [BL]H.00 matches network US, station DUG, BH? or LH? & loc 00\n");
        printf("         US[A-C]....BHZ.. match and US station starting with A, B or C BHZ only & all loc\n");
        printf("-delaz   [mindeg:]maxdeg:lat:long [-s regexp][-kml] return list of stations within deg of lat and long and option regexp \n");
        printf("-delazc  [mindeg:]maxdeg:lat:long [-s regexp][-kml] return list of channels within deg of lat and long and option regexp \n");
        printf("-help    Get help message from server (for expert mode)\n");
        printf("-u [um|nm] Units of responses are to be in nanometers or micrometers\n");
        printf("-xml     Output response in XML format.  May work for other options (not normally needed)\n");
        printf("-c [acdosr] pass command exactly (expert mode)\n");
        printf("-sac[:dirstub] Output preferred in SAC format unit=um.  If dirstub is present, each response will be in a separate file.\n");
        printf("-h nnn.nnn.nnn.nnn Use the given server rather than the default=%s (cwbpub)\n",host);
        printf("-p nnnn  Use port nnnn instead of the default server port number (2052)\n");
        exit(0);
    }
    /* set up defaults */
    req[0]=0;
    dirstub[0]=0;
    int kml=0;

     /* process the command line args */
     for(i = 1; i<argc; i++) {
         if(strcmp(argv[i], "-h") == 0) {
             strcpy(host,argv[i+1]);
             i++;
             if(dbg) fprintf(stderr, "Use host : %s\n",host);
         }
         else if(strcmp(argv[i],"-p") == 0) {
             port = atoi(argv[i+1]);
             i++;
             if(dbg) fprintf(stderr, "Use port : %d\n",port);
         }
         else if(strcmp(argv[i],"-dbg") == 0) {dbg=1; }
         else if(strcmp(argv[i],"-c") == 0) {
             strcat(req,"-c ");
             chr = *argv[i+1];
             strcat(req,argv[i+1]);
             if( !(chr == 'a' || chr == 'c' || chr == 'd' || chr == 'k' ||
                    chr == 'l' || chr == 'o' || chr == 's' || chr == 'r' )) {
                 printf("-c arguments must be a, c, d, l, o, r, or s\n");
                 exit(1);
             }
             strcat(req," ");
             i++;
         }
         else if(strcmp(argv[i],"-b") == 0 || strcmp(argv[i],"-e") == 0 ||
             strcmp(argv[i],"-s") == 0) {
             strcat(req, argv[i]);
             strcat(req, " ");
             strcat(req, replace(argv[i+1],' ','-'));
             strcat(req, " ");
             i++;
         }
         else if(strcmp(argv[i],"-help") == 0) {
             strcat(req,"-h ");
             break;
         }
         else if(strcmp(argv[i],"-station") == 0) {
             strcat(req,"-c s ");
         }
         else if(strcmp(argv[i],"-cooked") == 0) {
             strcat(req,"-c r ");
         }
         else if(strcmp(argv[i],"-orient") == 0) {
             strcat(req,"-c o ");
         }
         else if(strcmp(argv[i],"-coord") == 0) {
             strcat(req,"-c c ");
         }
         else if(strcmp(argv[i],"-lsc") == 0) {
             strcat(req,"-c l ");
         }
         else if(strcmp(argv[i],"-desc") == 0) {
             strcat(req,"-c d ");
         }
         else if(strcmp(argv[i],"-kml") == 0) {
             strcat(req,"-c k ");
             kml=1;
         }
         else if(strcmp(argv[i],"-alias") == 0) {
             strcat(req,"-c a ");
         }
         else if(strncmp(argv[i],"-dataless",9) == 0) {
             strcat(req,"-c r -dataless ");
             pnt = strchr(argv[i],':');
             if(pnt == NULL) strcpy(dirstub,"./");
             else strcpy(dirstub,pnt+1);
         }
         else if(strncmp(argv[i], "-sac",4) == 0) {
             strcat(req,"-sac ");
             pnt = strchr(argv[i],':');
             if(pnt == NULL) strcpy(dirstub,"./");
             else strcpy(dirstub,pnt+1);
         }
         else if(strncmp(argv[i],"-resp",5) == 0) {
             strcat(req,"-c r -resp ");
             pnt = strchr(argv[i],':');
             if(pnt == NULL) strcpy(dirstub,"./");
             else strcpy(dirstub,pnt+1);
         }
         else if(strcmp(argv[i], "-xml") == 0 ||
             strcmp(argv[i], "-allowwarn") == 0 || strcmp(argv[i], "-allowbad")  == 0)  {
             strcat(req, argv[i]);
             strcat(req, " ");
         }
         else if(strcmp(argv[i], "-forceupdate")  == 0){
             strcat(req, argv[i]);
             strcat(req, " ");
         }
         else if(strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "-icon") == 0 ||
             strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "-a") == 0 ||
             strcmp(argv[i],"-delaz") == 0 || strcmp(argv[i], "-delazc") == 0) {
             strcat(req, argv[i]);
             strcat(req, " ");
             strcat(req, argv[i+1]);
             strcat(req," ");
             if(strcmp(argv[i], "-a") == 0) strcat(req, "-c c ");
             i++;
         }
         else {
             printf("Bad argument at %d value=%s\n",i,argv[i]);
             exit(3);
         }
     }
     /* if no -c has been selected by the arguments, default to get responses */
     pnt = strstr(req, "-c");
     if(pnt == NULL) if(strstr(req,"-h") == NULL) strcat(req, "-c r ");
     if(dbg) fprintf(stderr, "req=%s|dirstub=%s|host=%s\n",req, dirstub, host);
        printf("%s",req);
	 /* open the socket to the server */
   if((hp=gethostbyname(host)) == NULL) {
      fprintf(stderr,"MDGET:  unknown host (%s)\n", host);
      exit(1);
   }

   if((bnfp=socket(AF_INET,SOCK_STREAM,0)) < 0) {
      perror("MDGET:  socket");
      exit(1);
   }
      printf("Here is the request");
   printf("%s",req);
   /*bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);*/
	 memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
   sin.sin_family = AF_INET;
   sin.sin_port = htons(port);
   
   if(connect(bnfp,(struct sockaddr *)&sin,sizeof(sin)) < 0) {
      perror("MDGET:  connect");
      close(bnfp);
 /*     exit(1)   ;  */
   }

   /* Send the request string. */
   strcat(req,"\n");
   l = write(bnfp, req, strlen(req));
   printf("Here is the request");
   printf("%s",req);
   if(l != strlen(req)) {
        perror("MDGET: write command");
        exit(1);
   }

    /* Read the response. */
    if(dirstub[0] == 0) {
        off=0;
    	while(l != -1) {
            l = recv(bnfp, &ans[off], 10240 - off, 0);
            if(l > 0) {
                j += l;
                ans[l]=0;
                if(strstr(ans,"** MetaDataServer not up") != NULL) {
                        fprintf(stderr," ** MetaDataServer not up.  Wait and try again\n");
                }
                pnt = strstr(ans,"<EOR>");
                if(pnt != NULL && kml) l = l-8;
                fwrite(ans, l, 1, stdout);
                if(dbg) fprintf(stderr, "l=%d j=%d\n", l, j);
                ans[l]=0;
                if(pnt != NULL) break;
            }
            else if (l == -1) {
                fprintf(stderr,"recv err bnfp=%d off=%d\n",bnfp,  off);
                perror("MDGET: Error reading response from server");
                exitstatus=1;
            }
            else {
                fprintf(stderr, "rcv return not handled <EOF?> l=%d\n",l);
                exitstatus=1;
                break;
            }
        }
    }
    else {
    /* this is more complicated.  We need to pick off file names when starting and
     then scan for EOE.  During the filename phase we may get a EOR and be ready to end
     */
    	filePhase=1;
        off=0;
        j=0;
        flen=0;
    	for(;;) {
            l = recv(bnfp, &ans[off], 10240 - off, 0);
            if(dbg) fprintf(stderr,"Read %d bytes.\n",l);
            if(l >= 0) {
                j += l;
                if(filePhase) {
                    ans[off+l]=0;			/* make sure this will behave as a string */
                    pnt = strstr(ans, " <EOR>");
                    if(pnt != NULL) {		/* we are done, do cleanup and leave */
                            write(out, ans, (pnt - ans - 1));
                            flen += (pnt -ans -1);
                            break;
                    }
                    /** is this SAC style input?*/
                    if(strncmp(ans, "* CHANNEL",9) == 0) {
                        pnt = strstr(ans,"\n"); /* need the whole line*/
                        if(pnt == NULL) continue;   /* need more of the line*/
                        strcpy(filename,dirstub);
                        if(filename[strlen(dirstub)-1] != '/') strcat(filename,"/");
                        m = strlen(filename);
                        for(k=15; k<27; k++) {
                            /* if channel is < 12 char you get the newline and * from next line, spaces too*/
                            if(ans[k] == ' ' || ans[k] == '\n' || ans[k] == '*' || ans[k] == '?') filename[m+k-15] = '_';
                            else filename[m+k-15] = ans[k];
                        }
                        filename[m+12]=0;
                        strcat(filename, ".sac.pz");
                        pnt = ans-1;        /* reset so all is put in the file*/
                    }
                    else {
                        pnt = strstr(ans, "\n");
                        /*if(dbg) fprintf(stderr,"dir first return l=%d pnt=%d ans=%d\n", l, pnt, &ans[off]);*/
                        if(pnt == NULL) {
                                printf("Not enough data for filename.  get more\n");
                                continue;
                        }
                        strcpy(filename,dirstub);
                        if(filename[strlen(dirstub)-1] != '/') strcat(filename,"/");
                        strncat(filename, ans, pnt-ans);
                    }
                    out = open(filename, O_WRONLY | O_CREAT |O_TRUNC, 0644);
                    /*if(dbg) fprintf(stderr,"Open file : %s| off=%d l=%d pnt=%d ans=%d\n",
                            filename, off, l, pnt, ans);*/
                    l = l+off - (pnt - ans +1);
                    if(l > 0) memcpy(ans, pnt+1, l);		/* move remaining data to beginning of buffer */
                    off=0;
                    if(dbg) fprintf(stderr,"aft l=%d off=%d 1st char=%c%c%c\n",l,off, ans[0],ans[1],ans[2]);
                    filePhase=0;
                }
                if(filePhase == 0 && (l+off) >= 5) {
                    ans[l+off]=0;
                    pnt = strstr(ans, " <EOE>");
                    if(pnt != NULL) {
                        write(out, ans, (pnt-ans - 1));
                        flen += (pnt -ans -1);
                        /*if(dbg) fprintf(stderr, "<EOE> detected at %d pnt=%d ans=%d l=%d\n",
                                (pnt-ans-1),pnt,ans,l);*/
                        printf("%s %d bytes.\n",filename,flen);
                        flen=0;
                        close(out);
                        l = l - (pnt - ans) -7;		/* this is l adjusted to be after of EOE>\n"*/
                        /*if(dbg) fprintf(stderr, "%d bytes out EOE detected at %d  new l=%d off=%d %c %c %c %c %c\n",
                                j, pnt,l, off, ans[l-5],ans[l-4],ans[l-3],ans[l-2], ans[l-3]);*/
                        if(l+off > 0) memcpy(ans, pnt+7, l+off);
                        if(dbg) fprintf(stderr, "%c%c%c%c%c%c%c\n",ans[0],ans[1],ans[2],ans[3],ans[4],ans[5],ans[6]);
                        off = l+off;
                        filePhase=1;
                        ans[off]=0;
                        pnt = strstr(ans, " <EOR>");
                        if(pnt != NULL) {		/* we are done, do cleanup and leave */
                            if(dbg) fprintf(stderr, "Found EOR after EOF\n");
                            break;
                        }
                    }
                    else {				/* its just more of the file write it out) less the last 5 bytes */
                        if(dbg) fprintf(stderr, "write to file %d\n",l+off-5);
                        write(out,ans, l+off-5);
                        flen += l+off-5;
                        memcpy(ans, &ans[l+off-5], 5);
                        off=5;
                    }
                }
            }			/* end if l > 0 */
            else if(l < 0) {	fprintf(stderr,"recv err getting filename after off=%d l=%d err=%s\n",
                        off,l,strerror(errno));
                perror("MDGET: Error reading response from server");
                exitstatus=1;
                break;
            }
            else {
                fprintf(stderr, "rcv return not handled <EOF?> l=%d\n",l);
                exitstatus=1;
                break;
            }
        }				/* for ever */
    }					/* else clause of if this is a file type request (dirsub[0]!=0 */
    if(dbg) fprintf(stderr, "total bytes %d\n",j);
    if(out >0) close(out);
/* Done. */
   close(bnfp);
	 if(dbg) fprintf(stderr, "Exitting normally\n");
	 fflush(stderr);
   exit(exitstatus);
}
char *replace(char *str, int chr, int with) {
    int i;
    /*printf("replace in=%s\n",str);*/
    for(i=0; i<strlen(str); i++)
            if(*(str+i) == chr) *(str+i)=with;
    /*printf("replace ot=%s\n",str);*/
    return str;
}
