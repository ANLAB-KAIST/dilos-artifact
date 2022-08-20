/*
 * Copyright (C) 2013-2014 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */


#include <sys/stat.h>
#include <sys/vfs.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <osv/mount.h>

#define BUF_SIZE    4096

int sys_mount(const char *dev, const char *dir, const char *fsname, int flags, void *data);

static int tests = 0, fails = 0;

static void report(bool ok, const char *msg)
{
    ++tests;
    fails += !ok;
    printf("%s: %s\n", (ok ? "PASS" : "FAIL"), msg);
}

#ifdef __OSV__
extern "C" int vfs_findroot(char *path, struct mount **mp, char **root);

int check_zfs_refcnt_behavior(void)
{
    struct mount *mp;
    char mount_path[] = "/";
    char file[64], newfile[64];
    int old_mcount;
    int i, fd, ret = 0;

    /* Get refcount value from the zfs mount point */
    ret = vfs_findroot(mount_path, &mp, (char **) &file);
    if (ret) {
        return -1;
    }
    old_mcount = mp->m_count;

    // Use mkstemp to capture path of a temporary file used later
    snprintf(file, 64, "%sfileXXXXXX", mount_path);
    fd = mkstemp(file);
    if (fd <= 0) {
        return -1;
    }
    close(fd);
    unlink(file);

    /* Create hard links, and remove them afterwards to exercise the refcount code */
    for (i = 0; i < 10; i++) {
        fd = open(file, O_CREAT|O_TRUNC|O_WRONLY|O_SYNC, 0666);
        if (fd <= 0) {
            return -1;
        }
        close(fd);
        unlink(file);
    }

    fd = open(file, O_CREAT|O_TRUNC|O_RDWR, 0666);
    if (fd <= 0) {
        return -1;
    }
    close(fd);

    snprintf(newfile, 64, "%snewfile", file);

    /* Create a link to file into newfile */
    ret = link(file, newfile);
    if (ret != 0) {
        return -1;
    }

    /*
     * Force EEXIST on destination path, so the refcnt of the underlying znode
     * will be bumped for each link below.
     * VOP_INACTIVE should be called by each vrele to release one refcnt of
     * the znode properly.
     */
    for (i = 0; i < 10; i++) {
        ret = link(file, newfile);
        if (ret != -1 || errno != EEXIST) {
            return -1;
        }
    }
    unlink(file);
    unlink(newfile);

    /* Get the new refcount value after doing strategical fs operations */
    ret = vfs_findroot(mount_path, &mp, (char **) &file);
    if (ret) {
        return -1;
    }

    /* Must be equal */
    return !(old_mcount == mp->m_count);
}
#endif

int main(int argc, char **argv)
{
#define TESTDIR    "/usr"
    struct statfs st;
    DIR *dir;
    char path[PATH_MAX];
    struct dirent *d;
    struct stat s;
    char foo[PATH_MAX] = {0};
    int fd, ret;

    report(statfs("/usr", &st) == 0, "stat /usr");

    printf("f_type: %ld\n", st.f_type);
    printf("f_bsize: %ld\n", st.f_bsize);
    printf("f_blocks: %ld\n", st.f_blocks);
    printf("f_bfree: %ld\n", st.f_bfree);
    printf("f_bavail: %ld\n", st.f_bavail);
    printf("f_files: %ld\n", st.f_files);
    printf("f_ffree: %ld\n", st.f_ffree);
    printf("f_namelen: %ld\n", st.f_namelen);
    printf("f_fsid [0]: %ld\n", st.f_fsid.__val[0]);
    printf("f_fsid [1]: %ld\n", st.f_fsid.__val[1]);

    report((dir = opendir(TESTDIR)), "open testdir");

    while ((d = readdir(dir))) {
        if (strcmp(d->d_name, ".") == 0 ||
            strcmp(d->d_name, "..") == 0) {
            printf("found hidden entry %s\n", d->d_name);
            continue;
        }

        snprintf(path, PATH_MAX, "%s/%s", TESTDIR, d->d_name);
        report((ret = stat(path, &s)) == 0, "stat file");
        if (ret < 0) {
            printf("failed to stat %s\n", path);
            continue;
        }

        report((ret = (S_ISREG(s.st_mode) || S_ISDIR(s.st_mode))),
               "entry must be a regular file");
        if (!ret) {
            printf("ignoring %s, not a regular file\n", path);
            continue;
        }

        printf("found %s\tsize: %ld\n", d->d_name, s.st_size);
    }

    report(closedir(dir) == 0, "close testdir");
    report(mkdir("/tmp/testdir", 0777) == 0, "mkdir /tmp/testdir (0777)");
    report(stat("/tmp/testdir", &s) == 0, "stat /tmp/testdir");

    fd = open("/tmp/foo", O_CREAT | O_TRUNC | O_WRONLY | O_SYNC, 0666);
    report(fd > 0, "create /tmp/foo");

    report(write(fd, &foo, sizeof(foo)) == sizeof(foo), "write sizeof(foo) bytes to fd");
    report(pwrite(fd, &foo, sizeof(foo), LONG_MAX) == -1 && errno == EFBIG, "check for maximum allowed offset");
    report(fsync(fd) == 0, "fsync fd");
    report(fstat(fd, &s) == 0, "fstat fd");

    printf("file size = %lld\n", s.st_size);

    report(statfs("/tmp", &st) == 0, "stat /tmp");
    dev_t f_fsid = ((uint32_t)st.f_fsid.__val[0]) | ((dev_t) ((uint32_t)st.f_fsid.__val[1]) << 32);
    report(f_fsid == s.st_dev, "st_dev must be equals to f_fsid");

    report(close(fd) == 0, "close fd");

    fd = creat("/tmp/foo", 0666);
    report(fd > 0, "possibly create /tmp/foo again");

    report(fstat(fd, &s) == 0, "fstat fd");
    printf("file size = %lld (after O_TRUNC)\n", s.st_size);
    report(close(fd) == 0, "close fd again");

    report(rename("/tmp/foo", "/tmp/foo2") == 0,
           "rename /tmp/foo to /tmp/foo2");

    report(rename("/tmp/foo2", "/tmp/testdir/foo") == 0,
           "rename /tmp/foo2 to /tmp/testdir/foo");

    report(unlink("/tmp/testdir/foo") == 0, "unlink /tmp/testdir/foo");

    report(rename("/tmp/testdir", "/tmp/testdir2") == 0,
           "rename /tmp/testdir to /tmp/testdir2");

    report(rmdir("/tmp/testdir2") == 0, "rmdir /tmp/testdir2");
#if 0
#ifdef __OSV__
    report(check_zfs_refcnt_behavior() == 0, "check zfs refcount consistency");
#endif
#endif

#if 0
    fd = open("/mnt/tests/tst-zfs-simple.c", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    memset(rbuf, 0, BUF_SIZE);
    ret = pread(fd, rbuf, BUF_SIZE, 0);
    if (ret < 0) {
        perror("pread");
        return 1;
    }
    if (ret < BUF_SIZE) {
        fprintf(stderr, "short read\n");
        return 1;
    }

    close(fd);

//  rbuf[BUF_SIZE] = '\0';
//  printf("%s\n", rbuf);
#endif

    // Report results.
    printf("SUMMARY: %d tests, %d failures\n", tests, fails);
    return 0;
}
