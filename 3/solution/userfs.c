#include "userfs.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

enum {
	BLOCK_SIZE = 512,
	MAX_FILE_SIZE = 1024 * 1024 * 1024,
};

/** Global error code. Set from any function on any error. */
static enum ufs_error_code ufs_error_code = UFS_ERR_NO_ERR;

struct block {
	/** Block memory. */
	char *memory;
	/** How many bytes are occupied. */
	int occupied;
	/** Next block in the file. */
	struct block *next;
	/** Previous block in the file. */
	struct block *prev;

	/* PUT HERE OTHER MEMBERS */
};

struct file {
	/** Double-linked list of file blocks. */
	struct block *block_list;
	/**
	 * Last block in the list above for fast access to the end
	 * of file.
	 */
	struct block *last_block;
	/** How many file descriptors are opened on the file. */
	int refs;
	/** File name. */
	const char *name;
	/** Files are stored in a double-linked list. */
	struct file *next;
	struct file *prev;

	/* PUT HERE OTHER MEMBERS */
	int assigned_for_deletion;
};

static struct file*
file_init(const char* name)
{
	struct file *f = malloc(sizeof(*f));
	f->name = name;
	f->refs = 0;
	f->block_list = NULL;
	f->last_block = NULL;
	f->next = NULL;
	f->prev = NULL;
	f->assigned_for_deletion = 0;

	return f;
}

static int
file_refs_dec(struct file *f)
{
	f->refs -= 1;
	return f->refs;
}

static int
file_refs_inc(struct file *f)
{
	f->refs += 1;
	return f->refs;
}

/**
 * Check if file could be deleted.
 * 
 * @param f pointer to file struct  
 * @return int Return 1 if no opened FD anymore 
 * and if ufs_delete() was called on it.
 */
static int
file_is_ready_to_delete(struct file *f)
{
	if (f->refs == 0 && f->assigned_for_deletion == 1)
	{
		return 1;
	}

	return 0;
}

static int
file_delete(struct file *f)
{
	struct block *temp = f->block_list;
	while (temp->next != NULL)
	{
		temp = temp->next;
		free(temp->prev);
	}
	free(temp);

	struct file *temp_file = f->prev;
	if (f->next != NULL)
	{
		f->next->prev = temp_file;
	}
	if (temp_file != NULL)
	{
		temp_file->next = f->next;
	}
	free(f);

	return 0;
}

/** List of all files. */
static struct file *file_list = NULL;

static int
file_list_append(struct file *f)
{
	if (file_list == NULL)
	{
		file_list = f;
	}
	else
	{
		struct file *temp = file_list;
		while (temp->next != NULL)
		{
			temp = temp->next;
		}
		temp->next = f;
		f->prev = temp;
	}

	return 0;
}

struct filedesc {
	struct file *file;

	/* PUT HERE OTHER MEMBERS */
	int block_number; 
	int block_offset;
};

/**
 * An array of file descriptors. When a file descriptor is
 * created, its pointer drops here. When a file descriptor is
 * closed, its place in this array is set to NULL and can be
 * taken by next ufs_open() call.
 */
static struct filedesc **file_descriptors = NULL;
static int file_descriptor_count = 0;
static int file_descriptor_capacity = 0;

/**
 * If FDT expanded before calling it
 * then function will always return first
 * free index and never return -1.
 * 
 * @return int first available index in FDT.
 */
static int
file_descriptors_get_available()
{
	for (int i = 0; i < file_descriptor_count; ++i)
	{
		if (file_descriptors[i] == NULL)
		{
			return i;
		}
	}

	return -1;
}

enum ufs_error_code
ufs_errno()
{
	return ufs_error_code;
}

int
ufs_open(const char *filename, int flags)
{
	ufs_error_code = UFS_ERR_NO_ERR;
	
	/**
	 * Check if file exists on the 'disk'
	 * (on the heap actually): in the file_list struct.
	 */
	if (file_list == NULL)
	{
		if (flags != UFS_CREATE)
		{
			ufs_error_code = UFS_ERR_NO_FILE;
			return -1;
		}
		else
		{
			/**
			 * Allocate memory for new descriptor in FDT.
			 */
			file_descriptor_count += 1;
			file_descriptor_capacity = file_descriptor_count * sizeof(struct filedesc*);
			file_descriptors = realloc(file_descriptors, file_descriptor_capacity);
			
			/**
			 * Allocate memory for filedesc struct.
			 * Allocate memory for file struct.
			 */
			struct filedesc *fd = malloc(sizeof(*fd));
			struct file *f = file_init(filename);
			fd->file = f;
			fd->block_number = 0; // WARNING MAYBE -1
			fd->block_offset = 0; // WARNING MAYBE -1
			
			/**
			 * Add entry to FDT.
			 */
			file_descriptors[0] = fd;

			/**
			 * Add entry to list of files.
			 */
			file_list_append(f);

			return 0;
		}
	}
	else
	{
		struct file *temp = file_list;
		int is_existed = 0;
		while (temp != NULL)
		{
			int is_match = strcmp(temp->name, filename);
			if (is_match == 0 && temp->assigned_for_deletion == 0)
			{
				is_existed = 1;
				break;
			}
			temp = temp->next;
		}

		if (is_existed == 0)
		{
			if (flags == UFS_CREATE)
			{
				/* TODO. Create entry in FDT */
				/* return new fd */

				/**
				 * Allocate memory for new descriptor in FDT.
				 */
				file_descriptor_count += 1;
				file_descriptor_capacity = file_descriptor_count * sizeof(struct filedesc*);
				file_descriptors = realloc(file_descriptors, file_descriptor_capacity);
				
				/**
				 * Allocate memory for filedesc struct.
				 * Allocate memory for file struct.
				 */
				struct filedesc *fd = malloc(sizeof(*fd));
				struct file *f = file_init(filename);
				fd->file = f;
				fd->block_number = 0; // WARNING MAYBE -1
				fd->block_offset = 0; // WARNING MAYBE -1

				/**
				 * Find first available index in FDT.
				 */
				int fd_index = file_descriptors_get_available();

				/**
				 * Add entry to FDT.
				 */
				file_descriptors[fd_index] = fd;

				/**
				 * Add entry to list of files.
				 */
				file_list_append(f);

				return fd_index;
			}
			else
			{
				ufs_error_code = UFS_ERR_NO_FILE;
				return -1;
			}
		}
		else
		{
			/**
			 * Allocate memory for new descriptor in FDT.
			 */
			file_descriptor_count += 1;
			file_descriptor_capacity = file_descriptor_count * sizeof(struct filedesc*);
			file_descriptors = realloc(file_descriptors, file_descriptor_capacity);

			/**
			 * Allocate memory for filedesc struct.
			 */
			struct filedesc *fd = malloc(sizeof(*fd));
			fd->file = temp;
			fd->block_number = 0;
			fd->block_offset = 0;

			/**
			 * Find first available index in FDT.
			 */
			int fd_index = file_descriptors_get_available();

			/**
			 * Add entry to FDT.
			 */
			file_descriptors[fd_index] = fd;

			return fd_index;
		}
	}

	ufs_error_code = UFS_ERR_NO_FILE;
	return -1;
}

ssize_t
ufs_write(int fd, const char *buf, size_t size)
{
	/* IMPLEMENT THIS FUNCTION */
	ufs_error_code = UFS_ERR_NOT_IMPLEMENTED;
	return -1;
}

ssize_t
ufs_read(int fd, char *buf, size_t size)
{
	/* IMPLEMENT THIS FUNCTION */
	ufs_error_code = UFS_ERR_NOT_IMPLEMENTED;
	return -1;
}

int
ufs_close(int fd)
{
	ufs_error_code = UFS_ERR_NO_ERR;
	
	if (fd >= file_descriptor_count)
	{
		ufs_error_code = UFS_ERR_BAD_FILE_DECSRIPTOR;
		return -1;
	}
	else
	{
		struct filedesc *file_desc = file_descriptors[fd];
		if (file_desc == NULL || file_desc->file == NULL)
		{
			ufs_error_code = UFS_ERR_BAD_FILE_DECSRIPTOR;
			return -1;
		}

		/**
		 * Decrease number of opened FDs.
		 * Check if it was last one.
		 */
		file_refs_dec(file_desc->file);
		if (file_is_ready_to_delete(file_desc->file))
		{
			file_delete(file_desc->file);
		}

		/**
		 * Decrease counter.
		 * Free memory of current FD.
		 * Set NULL for FD by this index.
		 */
		file_descriptor_count -= 1;
		free(file_desc);
		file_descriptors[fd] = NULL;
	}

	return 0;
}

int
ufs_delete(const char *filename)
{
	ufs_error_code = UFS_ERR_NO_ERR;

	struct file *f = file_list;

	while (f != NULL)
	{
		int is_match = strcmp(f->name, filename);
		if (f->assigned_for_deletion != 1 && is_match == 0)
		{
			f->assigned_for_deletion = 1;
			if (file_is_ready_to_delete(f))
			{
				file_delete(f);
			}

			return 0;
		}
		f = f->next;
	}

	ufs_error_code = UFS_ERR_NO_FILE;
	return -1;
}
