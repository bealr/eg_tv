#include "files.h"

#include <dirent.h>
#include <string.h>
#include <strings.h>

static int HasVideoExtension(const char *name)
{
    const char *ext = strrchr(name, '.');

    if (!ext)
        return 0;

    ext++;

    return
        !strcasecmp(ext, "mp4") ||
        !strcasecmp(ext, "avi") ||
        !strcasecmp(ext, "mov") ||
        !strcasecmp(ext, "mkv") ||
        !strcasecmp(ext, "webm");
}

void LoadVideos(Menu *menu)
{
    menu->count = 0;
    menu->selected = 0;
    menu->scroll = 0;

    DIR *dir = opendir("videos");

    if (!dir)
        return;

    struct dirent *entry;

    while ((entry = readdir(dir)))
    {
        if (entry->d_type != DT_REG)
            continue;

        if (!HasVideoExtension(entry->d_name))
            continue;

        strncpy(
            menu->entries[menu->count].name,
            entry->d_name,
            sizeof(menu->entries[0].name) - 1);

        menu->entries[menu->count].name[
            sizeof(menu->entries[0].name) - 1] = '\0';

        menu->count++;

        if (menu->count >= MAX_FILES)
            break;
    }

    closedir(dir);
}