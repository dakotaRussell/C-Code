#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h> 
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define PARTITION_TABLE 0x1BE
#define BOOT_MAGIC_NUMBER 0x80
#define PARTITION_TYPE 0x81
#define BYTE510 0x55
#define BYTE511 0xAA
#define MAGIC_NUMBER 0x4D5A
#define REVERSE_MAGIC_NUMBER 0x5A4D
#define INODE_SIZE 64
#define DIRECTORY_SIZE 64
#define SECTOR_SIZE 512 
#define SUPERBLOCK 1024
#define DIRECT_ZONES 7
#define FILE_NAME_SIZE 60

/* bit masks */
#define FILE_TYPE_MASK 0170000 
#define REG_FILE_MASK 0100000 
#define DIR_MASK 0040000 
#define OWN_RD_MASK 0000400  /* owner read permission */
#define OWN_WR_MASK 0000200 /* owner write permission */
#define OWN_EX_MASK 0000100 /* Owner execute permission */
#define G_RD_MASK 0000040 /* Group read permission */
#define G_WR_MASK 0000020 /* Group write permission */
#define G_EX_MASK 0000010 /* Group execute permission */
#define OTH_RD_MASK 0000004 /* Other read permission */
#define OTH_WR_MASK 0000002 /* Other write permission */
#define OTH_EX_MASK 0000001 /*Other execute permission */

typedef struct partEntry {
   uint8_t bootind; 
   uint8_t start_head;
   uint8_t start_sec;
   uint8_t start_cyl;
   uint8_t type;
   uint8_t end_head;
   uint8_t end_sec;
   uint8_t end_cyl;
   uint32_t lFirst;
   uint32_t size; 
} partEntry; 

typedef struct directory {
   uint32_t inodeNum; 
   const char name[FILE_NAME_SIZE]; 
} directory; 

typedef struct superblock {
   uint32_t ninodes; /* number of inodes in this filesystem */
   uint16_t pad1; /* make things line up properly */
   int16_t i_blocks; /* # of blocks used by inode bit map */
   int16_t z_blocks; /* # of blocks used by zone bit map */   
   uint16_t firstdata; /* number of first data zone */
   int16_t log_zone_size; /* log2 of blocks per zone */
   int16_t pad2; /* make things line up again */
   uint32_t max_file; /* maximum file size */
   uint32_t zones; /* number of zones on disk */
   int16_t magic; /* magic number */
   int16_t pad3; /* make things line up again */
   uint16_t blocksize; /* block size in bytes */
   uint8_t subversion; /* filesystem sub–version */
} superblock; 

typedef struct inode {
   uint16_t mode; /* mode */
   uint16_t links; /* number or links */
   uint16_t uid;
   uint16_t gid;
   uint32_t size;
   int32_t atime;
   int32_t mtime;
   int32_t ctime;
   uint32_t zone[DIRECT_ZONES];
   uint32_t indirect;
   uint32_t two_indirect;
   uint32_t unused;
} inode;

int partEntryOffset = -1; /* offset for partition table entry */
int subpartEntryOffset = -1; /* offset for subpartition table entry */
int partOffset = -1; /* offset for the start of the partition itself */
FILE *fp;
int inodeTableOffset = -1;
superblock sb;
int zonesize = -1; /* size of zone */
char *path = NULL; /* string containing the path */
inode *inodeArr; /*table of all inodes within current partition*/
int incrVerb = 0; /* indicates if verbosity level is increased*/

void printUsage() {
   printf("usage: minls [ -v ] [ -p num [ -s num ] ] imagefile [ path ]\n");
}

void checkStatus(int status) {
   if (status != 0) { /* system call failed */
      perror("ERROR");
      exit(EXIT_FAILURE);
   } 
}

void findPartition (char* imageFile, long part, long subPart) {
   int status = -1; 
   partEntry entry;
   unsigned char c; /* used to read in bytes */
    
   if (!fp) { /* if opening the ImageFile fails*/
      perror("ERROR"); 
      exit(EXIT_FAILURE); 
   }  
   
   /* check validity of bytes 510 and 511 in the boot sector */
   status = fseek(fp, 510, SEEK_SET);
   checkStatus(status);
   status = fread(&c, 1, 1, fp);
   if (status != 1) {
      perror("ERROR"); 
      exit(EXIT_FAILURE); 
   }
   
   if (c != BYTE510) {
      fprintf(stderr, "Invalid partition table.\n");
      fprintf(stderr, "Unable to open disk image \"%s\".\n", imageFile);
      exit(EXIT_FAILURE);
   }
   else { /* checking byte 511 signature if byte 510 is accurate*/
      status = fread(&c, 1, 1, fp);
      if (status != 1) {
         perror("ERROR"); 
         exit(EXIT_FAILURE); 
      }
      
      if (c != BYTE511) {
         fprintf(stderr, "Invalid partition table.\n");
         fprintf(stderr, "Unable to open disk image \"%s\".\n", imageFile);
         exit(EXIT_FAILURE);
      }
   }
   
   /* find the requested partition */
   partEntryOffset = PARTITION_TABLE + part * sizeof(partEntry);
   status = fseek(fp, partEntryOffset, SEEK_SET);
   checkStatus(status);
   
   /*read in the partition entry*/ 
   status = fread(&entry, 1, sizeof(partEntry), fp); 
   if (status != sizeof(partEntry)) {
      perror("ERROR");
      exit(EXIT_FAILURE);
   }   

   if (entry.type != PARTITION_TYPE) {
      fprintf(stderr, "Bad magic number. (0x%04x)\n", entry.type);
      fprintf(stderr, "This doesn't look like a MINIX filesystem.\n");
      exit(EXIT_FAILURE); 
   }

   /* storing the start of the partition*/
   partOffset = SECTOR_SIZE*entry.lFirst; 

   if (subPart >= 0) { /* find subpartition if one is provided */ 
      subpartEntryOffset = partOffset + PARTITION_TABLE + 
                           subPart * sizeof(partEntry);
      status = fseek(fp, subpartEntryOffset, SEEK_SET);
      checkStatus(status);

      status = fread(&entry,1,sizeof(partEntry), fp);
      if (status != sizeof(partEntry)) {
         perror("ERROR");
         exit(EXIT_FAILURE);
      }

      if (entry.type != PARTITION_TYPE) {
         fprintf(stderr, "Bad magic number. (0x%04x)\n", entry.type);
         fprintf(stderr, "This doesn't look like a MINIX filesystem.\n");
         exit(EXIT_FAILURE); 
      }
      
      /* storing the start of the subpartition*/
      partOffset = SECTOR_SIZE*entry.lFirst; 
   }
  
  /* if (entry.bootind != BOOT_MAGIC_NUMBER) {
      fprintf(stderr, "Bad magic number. (0x%04x)\n", entry.bootind);
      fprintf(stderr, "This doesn't look like a MINIX filesystem.\n");
      exit(EXIT_FAILURE);
   } */ 

   

}

void printSuperblock() {
   fprintf(stderr, "Superblock Contents:\nStored Fields:\n");
   fprintf(stderr, "%*s%*d\n", -15, "  ninodes", 7, sb.ninodes);
   fprintf(stderr, "%*s%*d\n", -15, "  iblocks", 7, sb.i_blocks);
   fprintf(stderr, "%*s%*d\n", -15, "  z_blocks", 7, sb.z_blocks);
   fprintf(stderr, "%*s%*d\n", -15, "  firstdata", 7, sb.firstdata);
   fprintf(stderr, "%*s%*d\n", -15, "  log_zone_size", 7, sb.log_zone_size);
   fprintf(stderr, "%*s%*d\n", -10, "  max_file", 12, sb.max_file);
   fprintf(stderr, "%*s%#*x\n", -15, "  magic", 7, sb.magic);
   fprintf(stderr, "%*s%*d\n", -15, "  zones", 7, sb.zones);
   fprintf(stderr, "%*s%*d\n", -15, "  blocksize", 7, sb.blocksize);
   fprintf(stderr, "%*s%*d\n", -15, "  subversion", 7, sb.subversion);   
}

void getSuperblock() {
   int status = -1; 
   if (partOffset == -1) {
      partOffset = 0;
   }
   
   status = fseek(fp, partOffset+SUPERBLOCK, SEEK_SET); 
   checkStatus(status); 
   
   status = fread(&sb, 1, sizeof(superblock), fp);
   
   /* ensure all expected bytes were read */
   if (status != sizeof(superblock)) {
      perror("ERROR"); 
      exit(EXIT_FAILURE); 
   }

   /* check magic number to ensure this is a valid MINIX file system */  
   if (sb.magic != MAGIC_NUMBER) {
      fprintf(stderr, "Bad magic number. (0x%04x)\n", sb.magic);
      fprintf(stderr, "This doesn't look like a MINIX filesystem.\n");
      exit(EXIT_FAILURE); 
   }
   zonesize = sb.blocksize << sb.log_zone_size;
}

void printFileInfo(inode curr, directory entry) {
   char perm[] = "----------";
   static int printPath = 1; /* only = 1 on the first call */
   int fileType = 0;
   int i; /* used for for-loop */

   if (((curr.mode & FILE_TYPE_MASK) & DIR_MASK) == DIR_MASK) {
      perm[0] = 'd';
   }

   if (printPath && (fileType == DIR_MASK)) {
      printf("%s:\n", path);
   }


   /* owner permissions */
   if ((curr.mode & OWN_RD_MASK) == OWN_RD_MASK) {
      perm[1] = 'r';
   }
   if ((curr.mode & OWN_WR_MASK) == OWN_WR_MASK) {
      perm[2] = 'w';
   }
   if ((curr.mode & OWN_EX_MASK) == OWN_EX_MASK) {
      perm[3] = 'x';
   }

   /* group permissions */
   if ((curr.mode & G_RD_MASK) == G_RD_MASK) {
      perm[4] = 'r';
   }
   if ((curr.mode & G_WR_MASK) == G_WR_MASK) {
      perm[5] = 'w';
   }
   if ((curr.mode & G_EX_MASK) == G_EX_MASK) {
      perm[6] = 'x';
   }

   /* other permissions */
   if ((curr.mode & OTH_RD_MASK) == OTH_RD_MASK) {
      perm[7] = 'r';
   }
   if ((curr.mode & OTH_WR_MASK) == OTH_WR_MASK) {
      perm[8] = 'w';
   }
   if ((curr.mode & OTH_EX_MASK) == OTH_EX_MASK) {
      perm[9] = 'x';
   }

   if (incrVerb && printPath) {
      printf("   uint16_t mode     0x%04x (%s)\n", perm);
      printf("   uint16_t links    %d\n", curr.links);
      printf("   uint16_t uid      %d\n", curr.uid);
      printf("   uint16_t gid      %d\n", curr.gid);
      printf("   uint32_t size     %d\n", curr.size);
      printf("   uint32_t atime    %d\n", curr.atime);
      printf("   uint32_t mtime    %d\n", curr.mtime);
      printf("   uint32_t ctime    %d\n", curr.ctime);
   }

   if (printPath && ((curr.mode & FILE_TYPE_MASK) == DIR_MASK)) {
      printf("%s:\n", path);
   }

   if (printPath && ((curr.mode & FILE_TYPE_MASK) == REG_FILE_MASK)) {
      printf("%9s%10d /%s\n", perm, curr.size, entry.name);
   }
   else {
      printf("%9s%10d %s\n", perm, curr.size, entry.name);
   }

   printPath = 0; 
}




/* prints the directory's contents; @param: directory's inode*/
void printDir(inode dirNode, directory entry) {
   int i, j, k, loop1, loop2, status;
   int itersLeft;
   directory *dirArr = malloc(zonesize);
   uint32_t *indirArr = malloc(zonesize);
   uint32_t *dblIndirArr = malloc(zonesize);
   inode currNd; /* used to print file information */
   directory *dirEntry = malloc(sizeof(directory));

   /* if the inode passed in is for a file, don't iterate through its dzones */
   if (entry.inodeNum && (((dirNode.mode & FILE_TYPE_MASK) & REG_FILE_MASK) 
      == REG_FILE_MASK)) {
      printFileInfo(dirNode, entry);
   }
   else {
      itersLeft = ceil(dirNode.size / (double)zonesize);
      /* direct zones */
      loop1 = itersLeft > DIRECT_ZONES ? DIRECT_ZONES : itersLeft;
      for (i = 0; i < loop1; i++) {
      /* seek to value found in direct zone * zonesize */
         status = fseek(fp, partOffset + dirNode.zone[i]*zonesize, SEEK_SET);
         checkStatus(status);
         /* read in directory array of size zonesize */ 
         status = fread(dirArr, 1, zonesize, fp);  
         if (status != zonesize) {
            perror("ERROR");
            exit(EXIT_FAILURE);
         }
         /* iterate through directory array to get directory entries */
         for (j = 0; j < zonesize / DIRECTORY_SIZE; j++) {
            memcpy(dirEntry, &dirArr[j], DIRECTORY_SIZE);
            /* if the file is valid, print its details */
            if (dirEntry->inodeNum != 0) { 
               currNd = inodeArr[dirEntry->inodeNum - 1];
               printFileInfo(currNd, *dirEntry); 
            }
         }
         itersLeft--;
      }
       
      /* indirect zones */
      /* if you still have zones left to look at, seek to indirect zone*/
      if (itersLeft) {
         /* seeking to the array of zones in the indirect zone */
         status = fseek(fp, partOffset + dirNode.indirect*zonesize, SEEK_SET);
         checkStatus(status);   
         /* read in an array of uint32_t of size zonesize */
         status = fread(indirArr, 1, zonesize, fp);
         if (status != zonesize) {
            perror("ERROR");
            exit(EXIT_FAILURE);
         }
          
         /*iterate through indirArr*/
         loop1 = itersLeft > zonesize/sizeof(uint32_t) ? 
                  zonesize/sizeof(uint32_t) : itersLeft;
         for (i = 0; i < loop1; i++) {
            /* seek to indirArr[i] * zonesize and read in dirArr */
            status = fseek(fp, partOffset + indirArr[i]*zonesize, SEEK_SET);
            checkStatus(status); 
            status = fread(dirArr, 1, zonesize, fp);
            if (status != zonesize) {
               perror("ERROR");
               exit(EXIT_FAILURE);
            }
             
            /* iterate through dirArr to get directory entries */
            for (j = 0; j < zonesize / DIRECTORY_SIZE; j++) {
               dirEntry = &dirArr[j];
               /* if the file is valid, print its details */
               if (dirEntry->inodeNum != 0) { 
                  currNd = inodeArr[dirEntry->inodeNum - 1];
                  printFileInfo(currNd, *dirEntry);
               }
            }
            itersLeft--; 
         }
      }
          
      /* doubly indirect */
      /* if you still have zones to look at, seek to double direct zones */  
      if (itersLeft) {
         /* seek to array of double indirect zones */
         status = fseek(fp, partOffset + dirNode.two_indirect*
                  zonesize, SEEK_SET);
         checkStatus(status);
          
         /* read in array of double indirect zones */
         status = fread(dblIndirArr, 1, zonesize, fp);
         if (status != zonesize) {
            perror("ERROR");
            exit(EXIT_FAILURE);
         }
          
         /* iterate through double indirect zones :: LOOP 1 */
         loop1 = (itersLeft > zonesize/sizeof(uint32_t) ? 
             zonesize/sizeof(uint32_t) : itersLeft);
         for (i = 0; i < loop1; i++) {
            /* in each iteration, seek to indirect zone 
             * array provided in dblIndirArr[i]*/
            status = fseek(fp, partOffset + dblIndirArr[i]*zonesize, SEEK_SET); 
            checkStatus(status); 
            /* in each iteration, read in array of indirect zones */
            status = fread(indirArr, 1, zonesize, fp);
            if (status != zonesize) {
               perror("ERROR");
               exit(EXIT_FAILURE);
            }
            /* iterate through the indirect zones :: LOOP 2 */
            loop2 = (itersLeft > zonesize/sizeof(uint32_t) ? 
               zonesize/sizeof(uint32_t) : itersLeft);
            for (j = 0; j < loop2; j++) {
               /* in each iteration, seek to indirect zone array*/
               status = fseek(fp, partOffset + indirArr[j]*zonesize, SEEK_SET);
               checkStatus(status);
               /* in each iteration, read in array of directories in dirArray */
               status = fread(dirArr, 1, zonesize, fp);
               if (status != zonesize) {
                  perror("ERROR");
                  exit(EXIT_FAILURE);
               }                
               /* iterate through the dirArray to get directory entries */
               for (k = 0; k < zonesize/DIRECTORY_SIZE; k++) {
                  dirEntry = &dirArr[j];
                  /* if the file is valid, print its details */
                  if (dirEntry->inodeNum != 0) { 
                     currNd = inodeArr[dirEntry->inodeNum - 1];
                     printFileInfo(currNd, *dirEntry);
                  }            
              }   
            }         
         }         
      }         
   }
}


void findInode() {
   int status = -1;
   int i, j, k;
   int itersLeft; /* total zones to iterate through */
   char delim[] = "/";
   directory *dirArr = malloc(zonesize);
   uint32_t *indirArr = malloc(zonesize);
   uint32_t *dblIndirArr = malloc(zonesize);   
   char *token = malloc(100);
   int index; /*index for inodeArr; default = 1 (root)*/
   int tokenFound = 0;
   directory *dirEntry = calloc(1, sizeof(directory));
   int loop1, loop2; /* variables controlling the loop iterations */
   inodeArr = malloc(sb.ninodes*sizeof(inode)); 
   
   /* skip over blocks before inodes */
   inodeTableOffset = partOffset + sb.blocksize*
                      (sb.i_blocks + sb.z_blocks + 2); 
   status = fseek(fp, inodeTableOffset, SEEK_SET);
   checkStatus(status);
   
   status = fread(inodeArr, sizeof(inode), sb.ninodes, fp);
   if (status != sb.ninodes) {
      printf("status: %d\n", status);
      perror("ERROR"); 
      exit(EXIT_FAILURE);   
   } 
   /* get the first token */
   token = strtok(path, delim);
   index = 0;
   

   /* walk through other tokens */
   while (token != NULL) {
      itersLeft = ceil(inodeArr[index].size / (double)zonesize); 
      tokenFound = 0;
      
      /* direct zones */
      loop1 = itersLeft > DIRECT_ZONES ? DIRECT_ZONES : itersLeft;
      for (i = 0; i < loop1; i++) {
         /* seek to value found in direct zone * zonesize */
         status = fseek(fp, partOffset + inodeArr[index].zone[i]*
                  zonesize, SEEK_SET); 
         checkStatus(status);
         /* read in directory array of size zonesize */
         status = fread(dirArr, 1, zonesize, fp);  
         if (status != zonesize) {
            perror("ERROR");
            exit(EXIT_FAILURE);
         }
         /* iterate through directory array */
         for (j = 0; j < (zonesize / DIRECTORY_SIZE); j++) {
            /* directory name found */
            if (strcmp(dirArr[j].name, token) == 0) {
               tokenFound = 1;
               dirEntry = &dirArr[j];
               index = dirEntry->inodeNum - 1; /* update inode index */
               if (index == -1) {
                  perror("Invalid inode number: 0.\n");
               }
               break;
             }
          }
          itersLeft--;

          if (tokenFound) {
             break;
          }
       }
       
       /* indirect zones */
       /* if not found && you still have zones left to look at
        * seek to indirect zone*/
       if (!tokenFound && itersLeft) {
          /* seeking to the array of zones in the indirect zone */
          status = fseek(fp, partOffset + inodeArr[index].indirect*
                   zonesize, SEEK_SET);
          checkStatus(status);   
          /* read in an array of uint32_t of size zonesize */
          status = fread(indirArr, 1, zonesize, fp);
          if (status != zonesize) {
             perror("ERROR");
             exit(EXIT_FAILURE);
          }
          
          /*iterate through indirArr*/
          loop1 = itersLeft > zonesize/sizeof(uint32_t) ? 
                  zonesize/sizeof(uint32_t) : itersLeft;
          for (i = 0; i < loop1; i++) {
             /* seek to indirArr[i] * zonesize and read in dirArr */
             status = fseek(fp, partOffset + indirArr[i]*zonesize, SEEK_SET);
             checkStatus(status); 
             status = fread(dirArr, 1, zonesize, fp);
             if (status != zonesize) {
                perror("ERROR");
                exit(EXIT_FAILURE);
             }
             
             /* iterate through dirArr searching for file name */
             for (j = 0; j < zonesize / DIRECTORY_SIZE; j++) {
                /* directory name found */
                if (strcmp(dirArr[j].name, token) == 0) { 
                   tokenFound = 1;
                   dirEntry = &dirArr[j];
                   index = dirEntry->inodeNum - 1; /* update inode index */
                   if (index == -1) {
                      perror("Invalid inode number: 0.\n");
                   }
                   break;
                } 
             }
             itersLeft--; 
          }
       }
          
       /* doubly indirect */
       /* if still not found && inode[index].size > DIRECT_ZONES*
        * zonesize, seek to double direct zones */  
       if (!tokenFound && itersLeft) {
          /* seek to array of double indirect zones */
          status = fseek(fp, partOffset + inodeArr[index].two_indirect*
                   zonesize, SEEK_SET);
          checkStatus(status);
          
          /* read in array of double indirect zones */
          status = fread(dblIndirArr, 1, zonesize, fp);
          if (status != zonesize) {
             perror("ERROR");
             exit(EXIT_FAILURE);
          }
          
          /* iterate through double indirect zones :: LOOP 1 */
          loop1 = (itersLeft > zonesize/sizeof(uint32_t) ? 
              zonesize/sizeof(uint32_t) : itersLeft);
          for (i = 0; i < loop1; i++) {
             /* in each iteration, seek to indirect zone array 
              * provided in dblIndirArr[i]*/
             status = fseek(fp, partOffset + dblIndirArr[i]*
                      zonesize, SEEK_SET); 
             checkStatus(status); 
             /* in each iteration, read in array of indirect zones */
             status = fread(indirArr, 1, zonesize, fp);
             if (status != zonesize) {
                perror("ERROR");
                exit(EXIT_FAILURE);
             }
             /* iterate through the indirect zones :: LOOP 2 */
             loop2 = (itersLeft > zonesize/sizeof(uint32_t) ? 
                zonesize/sizeof(uint32_t) : itersLeft);
             for (j = 0; j < loop2; j++) {
                /* in each iteration, seek to indirect zone array*/
                status = fseek(fp, partOffset + indirArr[j]*
                         zonesize, SEEK_SET);
                checkStatus(status);
                status = fread(dirArr, 1, zonesize, fp);
                if (status != zonesize) {
                   perror("ERROR");
                   exit(EXIT_FAILURE);
                }
                /* iterate through the dirArray searching for token */
                for (k = 0; k < zonesize/DIRECTORY_SIZE; k++) {
                   /* directory name found */
                   if (strcmp(dirArr[k].name, token) == 0) { 
                      tokenFound = 1;
                      dirEntry = &dirArr[k];
                      index = dirEntry->inodeNum; /* update inode index */
                      if (index == 0) {
                         perror("Invalid inode number: 0.\n");
                      }
                      break;
                   }                  
                }
             }
             
          }         
       }
       /* else, your token is not in this directory */
       if (!tokenFound) {
          fprintf(stderr, "Could not find directory '%s'.\n", token);
          exit(EXIT_FAILURE);
       }
       token = strtok(NULL, path); /* advance to next path entry */
    }
    printDir(inodeArr[index], *dirEntry);
}


int main(int argc, char **argv) {
   long part = -1; /*partition value; negative if none provided by user*/
   long subPart = -1; /*subpartition value; negative if none provided by user*/
   int argsLeft = argc - 1;
   char *imageFile = NULL;
   char *ptr; /* used as strtol param */
   int c;
   
   /*parsing through the commandline and error checking if missing arguments*/
   while ((c = getopt (argc, argv, "p:s:v")) != -1) {
      switch (c) {
         case 'p':
            argsLeft--;
            argsLeft--;
            part = strtol(optarg, &ptr, 10); 
            break;
         case 's':
            argsLeft--;
            argsLeft--;
            subPart = strtol(optarg, &ptr, 10);
            break;
         case 'v':
            argsLeft--;
            incrVerb = 1;
            break;
         default:
            printUsage();
            exit(EXIT_FAILURE);
            break;
      }
   }
 
   /* only the imageFile is provided; default path to root */ 
   if (argsLeft == 1) { 
      imageFile = argv[argc-1];
      path = "/";
   }
   /*provided the imageFile first and then the file path */
   else if (argsLeft == 2){ 
      imageFile = argv[argc-2];
      path = argv[argc-1];
   }
   else { /* either no imageFile provided or too many arguments */
      printUsage();
      exit(EXIT_FAILURE);
   }

   fp = fopen(imageFile, "r");

   /* if partition number was passed in, call findPartition() */
   if (part != -1) {
      findPartition(imageFile, part, subPart);
   }

   /* always find getSuperblock() */ 
   getSuperblock();  
   if (incrVerb) { 
      printSuperblock();
   }
   findInode();
 
   return 0;
   
}

