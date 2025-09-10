/*------------------------------------------------------------------+
 *     PHY WIRELESS TECHNOLOGIES PROPRIETARY AND CONFIDENTIAL       |
 *       Copyright (C) Acorn Technologies, Inc.  2015-2022          |
 *                   All Rights Reserved                            |
 *------------------------------------------------------------------+
 *
 * otdoa_fat_fs.c
 *
 */

#if OTDOA_LITTLE_FS == 0

#include <zephyr/kernel.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <zephyr/fs/fs.h>
#include <ff.h>
#include <zephyr/storage/disk_access.h>


#define TEST_FS_MNTP    "/SD:"
static const char *disk_mount_pt = TEST_FS_MNTP;
#define BUFF_SIZE   1024
char test_buf[BUFF_SIZE];


static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
    .type = FS_FATFS,
    .fs_data = &fat_fs,
};





#if 1
//  The workers over in otdoa_file.c
extern int my_read(int file, char *buf, int nbytes);
extern int my_write(int file, const void *buf, int nbytes);
extern int my_open(const char *name, int mode);
extern int my_close(int file);
extern int my_lseek(int file, int ptr, int dir);
extern int _read(int file, char *buf, int nbytes);
extern int _write(int file, const void *buf, int nbytes);
extern int _open(const char *name, int mode);
extern int _close(int file);
extern int _lseek(int file, int ptr, int dir);
#endif

/*
*  Note the fatfs library is able to mount only strings inside _VOLUME_STRS
*  in ffconf.h
*/



int mount_fat_fs()
{
    mp.mnt_point = disk_mount_pt;
    int iMount = fs_mount(&mp);
    if (iMount == FR_OK)
    {
        printk("PASS fs_mount. %s\n", TEST_FS_MNTP);
        return 0;
    }
    else
    {
        printk("FAIL fs_mount %s.\n", TEST_FS_MNTP);
        return -1;
    }
}




void helper_sf_return(int iRet, char * szContext)
{
    printk("%s: ", szContext);
    switch (iRet)
    {
    case 0:         printk("success\n");  break;
    case EINVAL:    printk("when a bad file name is given\n");  break;
    case EROFS:     printk("when opening read - only file for write, or attempting to create a file on a system that has been mounted with the FS_MOUNT_FLAG_READ_ONLY flag\n");  break;
    case ENOENT:    printk("when the file path is not possible(bad mount point\n");  break;
    default:        printk("an other negative errno code, depending on a file system back-end.\n");  break;
    }
}

#define TEST_FILE   TEST_FS_MNTP"/FILE1.TXT"
int test_fs_open_read_close()
{
    int iReturn = 0;

    static struct fs_file_t filep;
    fs_file_t_init(&filep);
    //  Open
    iReturn = fs_open(&filep, TEST_FILE, FS_O_READ);
    helper_sf_return(iReturn, "test_fs_open_read_close()");
    if (0 != iReturn)
        printk("FAIL fs_open file\n");
    if (0 == iReturn)
    {
        printk("PASS fs_open:%s\n", TEST_FILE);
        //  Read
        memset(test_buf, 0, BUFF_SIZE);
        ssize_t brw = fs_read(&filep, test_buf, BUFF_SIZE);
        if (brw <= 0)
            printk("FAIL read file\n");
        else
            printk("PASS read file\n");
        printk("test_fs_open_read_close: fs_read:\"%s\"\n\n", test_buf);
        //  Close
        iReturn = fs_close(&filep);
        if (0 != iReturn)
            printk("FAIL fs_close file\n");
        else
            printk("PASS fs_close file\n");
    }
    return iReturn;
}

#define TEST_COUNTER_FILE   TEST_FS_MNTP"/COUNTER.BIN"
int test_fs_counter()
{
    int iReturn = 0;
    static struct fs_file_t filep;
    fs_file_t_init(&filep);

    //  Open or Create the counter file
    iReturn = fs_open(&filep, TEST_COUNTER_FILE, FS_O_RDWR | FS_O_CREATE);
    if (0 != iReturn)
    {
        printk("test_fs_counter: FAIL open\n");
        return iReturn;
    }
    printk("test_fs_counter: PASS fs_open:%s\n", TEST_COUNTER_FILE);
    //  Read the file
    int nCount = 0;     //  Count to 0
    ssize_t brw = 0;
    brw = fs_read(&filep, &nCount, sizeof(nCount));
    if (brw < 0)
        printk("test_fs_counter: FAIL fs_read\n");
    else
        printk("test_fs_counter: PASS fs_read\n");
    printk("test_fs_counter: Data read nCount=%d\n", nCount);

    //  Seek back the the beginning of the file
    iReturn = fs_seek(&filep, 0, FS_SEEK_SET);
    if (0 != iReturn)
    {
        printk("test_fs_counter: FAIL fs_seek [%d]\n", iReturn);
        fs_close(&filep);
        return iReturn;
    }
    printk("test_fs_counter: PASS fs_seek [%d]\n", iReturn);

    //  Increment the counter and write
    nCount++;
    brw = fs_write(&filep, (char *)&nCount, sizeof(nCount));
    if (brw < 0)
    {
        printk("test_fs_counter: FAIL fs_write [%zd]\n", brw);
        fs_close(&filep);
        return brw;
    }
    printk("test_fs_counter: PASS writing to file [%zd]\n", brw);
    printk("test_fs_counter: Data written nCount=%d\n", nCount);

    //  Close the file
    iReturn = fs_close(&filep);
    if (0 != iReturn)
        printk("test_fs_counter: FAIL fs_close file\n");
    else
        printk("test_fs_counter: PASS fs_close file\n");

    //  Test unlink by deleting the file if the count if over 5
    if (5 < nCount)
    {
        printk("test_fs_counter: PASS nCount is 5 - unlink\n");
        iReturn = fs_unlink(TEST_COUNTER_FILE);
        if (0 != iReturn)
            printk("test_fs_counter: FAIL fs_unlink\n");
        else
            printk("test_fs_counter: PASS fs_unlink\n");
    }
    return iReturn;
}

static int test_fs_lsdir(const char *path)
{
    int res;
    struct fs_dir_t dirp;
    static struct fs_dirent entry;

    fs_dir_t_init(&dirp);

    /* Verify fs_opendir() */
    res = fs_opendir(&dirp, path);
    if (res)
    {
        printk("test_fs_lsdir: Error opening dir %s [%d]\n", path, res);
        return res;
    }

    printk("\nListing dir %s ...\n", path);
    for (;;)
    {
        /* Verify fs_readdir() */
        res = fs_readdir(&dirp, &entry);

        /* entry.name[0] == 0 means end-of-dir */
        if (res || entry.name[0] == 0)
        {
            break;
        }

        if (entry.type == FS_DIR_ENTRY_DIR)
        {
            printk("[DIR ] %s\n", entry.name);
        }
        else
        {
            printk("[FILE] %s (size = %zu)\n", entry.name, entry.size);
        }
    }

    /* Verify fs_closedir() */
    fs_closedir(&dirp);

    return res;
}

int test_raw_io()
{
    int iReturn = 0;
    do
    {
        static const char *disk_pdrv = "SD";
        uint64_t memory_size_mb;
        uint32_t block_count;
        uint32_t block_size;
        printk(">>>>>Initialize storage %s.\n", disk_pdrv);
        if (disk_access_init(disk_pdrv) != 0)
        {
            printk(">>>>>Storage init ERROR!\n");
            break;
        }

        if (disk_access_ioctl(disk_pdrv, DISK_IOCTL_GET_SECTOR_COUNT, &block_count))
        {
            printk(">>>>>Unable to get sector count\n");
            break;
        }
        printk(">>>>>Block count %u\n", block_count);

        if (disk_access_ioctl(disk_pdrv, DISK_IOCTL_GET_SECTOR_SIZE, &block_size))
        {
            printk(">>>>>Unable to get sector size\n");
            break;
        }
        printk(">>>>>Sector size %u\n", block_size);

        memory_size_mb = (uint64_t)block_count * block_size;
        printk(">>>>>Memory Size(MB) %u\n", (uint32_t)(memory_size_mb >> 20));
    } while (0);

    return iReturn;
}



int test_otdoa_file()
{
    int iReturn = 0;

    int fd1 = my_open(TEST_FILE, FS_O_READ);
    if (fd1 < 0)
    {
        printk("test_otdoa_file: my_open error\n");
        return -1;
    }
    else
        printk("test_otdoa_file: my_open=%d\n", fd1);

    //  Read
    memset(test_buf, 0, sizeof(test_buf));
    int nRead = my_read(fd1, test_buf, sizeof(test_buf)-1);
    if (nRead < 0)
        printk("test_otdoa_file: read error=%d\n", nRead);

    printk("test_otdoa_file: read(%d, buf, %d). returned that %d bytes were read.\n", fd1, sizeof(test_buf), nRead);
    printk("test_otdoa_file: read \"%s\"\n", test_buf);

    // Using close system Call
    if (my_close(fd1) < 0)
    {
        printk("test_otdoa_file: my_close error c1\n");
        return -1;
    }
    printk("test_otdoa_file: closed the fd.\n");
    return iReturn;
}

int test_clib_file()
{
    int iReturn = 0;

//#define O_RDONLY    FS_O_READ
    int fd1 = open(TEST_FILE, FS_O_READ);   //BUGBUG: UNIX-style IO is using proprietary flag values!
    if (fd1 < 0)
    {
        printk("test_clib_file: open error %d, errn=%d\n", fd1, errno);
        return -1;
    }
    else
        printk("test_clib_file: opened the fd = % d\n", fd1);

    //  Read
    memset(test_buf, 0, sizeof(test_buf));
    int nRead = read(fd1, test_buf, sizeof(test_buf));
    if (nRead < 0)
        printk("test_clib_file: read error=%d\n", nRead);

    printk("test_clib_file: read(%d, buf, %d). returned that %d bytes were read.\n", fd1, sizeof(test_buf), nRead);
    printk("test_clib_file: read \"%s\"\n", test_buf);

    // Using close system Call
    if (_close(fd1) < 0)    //BUGBUG: Must call _close becasue cloase gets linked to z_impl_zsock_close
    {
        printk("test_clib_file: close error c1\n");
        return -1;
    }
    printk("test_clib_file: closed the fd.\n");
    return iReturn;
}

//  Read a file
int test_fread(const char * filename)
{
    int iReturn = 0;
    static char buf[1024];
    FILE *fpr = 0;
    printk("test_fread: BEGIN -------------------- %s --------------------\n", filename);
    fpr = fopen(filename, "r");
    if (fpr)
    {
// nriedel        for (size_t nRead = fread(buf, 1, sizeof(buf), fpr); nRead; nRead = fread(buf, 1, sizeof(buf), fpr))
        for (size_t nRead = fread(buf, 1, sizeof(buf)-1, fpr); nRead; )
        {
            // nriedel : null-terminate
            buf[nRead] = 0;
            printf("%s", buf);
            nRead = fread(buf, 1, sizeof(buf), fpr);
        }

        fclose(fpr);
    }
    printk("test_fread:   ENG -------------------- %s --------------------\n", filename);
    return iReturn;
}


//  Write a line to a file
int test_fwrite(const char * filename, const char * mode)
{
    int iReturn = 0;
    FILE *fpr = 0;
    fpr = fopen(filename, mode);
    if (fpr)
    {
        fprintf(fpr, "A line of text\n");
        fclose(fpr);
    }
    return iReturn;
}

int test_fopen_modes()
{
    char * szPath[] =
    {
        "/SD:/FILE1.TXT",
        "/SD:/FILE4.TXT",
        "/FILE5.TXT",
        0
    };

    int i = 0;

    static FILE *fpr = 0;

    char * szModes[] =
    {
        "r",    //  open for reading (The file must exist)
        "rb",   //  open for reading (The file must exist)
        "r+",   //  open for reading and writing (The file must exist)
        "rb+",  //  open for reading and writing (The file must exist)
        0
    };
    printk("test_fopen_modes: (READ) Open (fopen) %s\n", szPath[i]);
    for (int iMode = 0; szModes[iMode]; iMode++)
    {
        printf("test_fopen_modes: Mode=%s\n", szModes[iMode]);
        fpr = fopen(szPath[i], szModes[iMode]);
        if (fpr)
            fclose(fpr);
    }

    //  Test Write Modes
    char * szModesDestructive[] =
    {
        "w",    //  open for writing (creates file if it doesn't exist). Deletes content and overwrites the file.
        "wb",   //  open for writing (creates file if it doesn't exist). Deletes content and overwrites the file.
        "w+",   //  open for reading and writing. If file exists deletes content and overwrites the file, otherwise creates an empty new file
        "wb+",  //  open for reading and writing. If file exists deletes content and overwrites the file, otherwise creates an empty new file
        "a",    //  open for appending (creates file if it doesn't exist)
        "ab",   //  open for appending (creates file if it doesn't exist)
        "a+",   //  open for reading and writing (append if file exists)
        "ab+",  //  open for reading and writing (append if file exists)
        0
    };

    i = 2;
    printk("test_fopen_modes: (WRITE) Open (fopen) %s\n", szPath[i]);
    for (int iMode = 0; szModesDestructive[iMode]; iMode++)
    {
        printf("test_fopen_modes: Mode=%s\n", szModesDestructive[iMode]);
        for (int iLine = 0; iLine < 3; iLine++)
        {
            test_fwrite(szPath[i], szModesDestructive[iMode]);
            test_fread(szPath[i]);
        }
        fs_unlink("/SD:/FILE5.TXT");
    }

    return 1;
}

//  Test fopen, fclose, fread,
int test_file_io()
{
    int iReturn = 0;

    static FILE *fpr = 0;

    //  Try CLIB file IO
    char * szPath[] =
    {
        //      "SD:/FILE1.TXT",
        //  "/SD/FILE1.TXT",
        "/SD:/FILE1.TXT",
        0
    };
    for (int i = 0; szPath[i]; i++)
    {
        //  Opening the file in "r" mode
        fpr = fopen(szPath[i], "r");
        if (fpr)
        {
            printk("PASS: Open (fopen) %s, file*=%p\n", szPath[i], fpr);
            //  Read
            memset(test_buf, 0, sizeof(test_buf));
            int iFR = fread(test_buf, 1, sizeof(test_buf), fpr);
            if (iFR > 0)
                printk("PASS: fread=%d\n", iFR);
            else
                printk("FAIL: fread=%d, errno=%d\n", iFR, errno);
            printk("test_file_io: fread(buf=%p, size=%d, count=%d, fpr=%p). returned that %d bytes were read.\n", test_buf, 1, sizeof(test_buf), fpr, iFR);
            printk("test_file_io: fread \"%s\"\n", test_buf);
            printk("test_file_io: Close %s\n", szPath[i]);
            fclose(fpr);
        }
        else
        {
            printk("FAIL: Open %s\n", szPath[i]);
        }
    }
    return iReturn;
}


int test_fs_raw()
{
    int iReturn = 0;

    //BEGIN - Test FAT FS Inbtegration
    // raw disk i/o
    int iTR = test_raw_io();
    if (iTR == FR_OK)
    {
        printk("PASS test_raw_io. %s\n", TEST_FS_MNTP);
    }
    else
    {
        printk("FAIL test_raw_io %s.\n", TEST_FS_MNTP);
        //      goto pots_fs;
    }

    return iReturn;
}

int test_fs()
{
    int iReturn = 0;
    test_fopen_modes();
    //  Test fs_open, fs_close, fs_read
    int iT = test_fs_open_read_close();
    if (0 == iT)
        printk("PASS test_fs_open_read_close.\n");
    else
        printk("FAIL test_fs_open_read_close.\n");

    //  Test fs_open, fs_close, fs_read, fs_write, fs_seek
    int iTC = test_fs_counter();
    if (0 == iTC)
        printk("PASS test_fs_counter.\n");
    else
        printk("FAIL test_fs_counter.\n");

    //  Test the fs_ warppers using their internal names
    int iOF = test_otdoa_file();
    if (0 == iOF)
        printk("PASS test_otdoa_file().\n");
    else
        printk("FAIL test_otdoa_file().\n");

    //  Test the fs_ warppers using their CLIB names (_open,_close,_read,_write,_seek)
    int iCL = test_clib_file();
    if (0 == iCL)
        printk("PASS test_clib_file().\n");
    else
        printk("FAIL test_clib_file().\n");

    //  Test the fs_ warppers using their CLIB names (_open,_close,_read,_write,_seek)
    int iFIO = test_file_io();
    if (0 == iFIO)
        printk("PASS test_file_io().\n");
    else
        printk("FAIL test_file_io().\n");

    //  List the root directory
    test_fs_lsdir(disk_mount_pt);
    //END - Test FAT FS Inbtegration

    return iReturn;
}

#endif // OTDOA_LITTLE_FS == 0

