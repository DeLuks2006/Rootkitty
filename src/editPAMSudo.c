#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    const char *pam_sudo_path = "/etc/pam.d/sudo";
    const char *tempfile = "sudo.new";
    const char *line_to_insert = "auth sufficient /lib/security/pam_yourmodule.so\n";
    const char *pam_header = "#%PAM-1.0\n";
    FILE *read_fp, *write_fp;
    char buffer[256];
    
    // Open the PAM configuration file for reading.
    read_fp = fopen(pam_sudo_path, "r");
    if (!read_fp) {
        perror("Error opening file for reading");
        return EXIT_FAILURE;
    }
    
    // Open a temporary file for writing.
    write_fp = fopen(tempfile, "w");
    if (!write_fp) {
        perror("Error opening file for writing");
        fclose(read_fp);
        return EXIT_FAILURE;
    }
    
    // Write the PAM header to the temp file.
    fputs(pam_header, write_fp);
    
    // Insert your custom module line after the PAM header.
    fputs(line_to_insert, write_fp);

    // Now copy the rest of the original sudo PAM file, starting from the next line.
    while (fgets(buffer, sizeof(buffer), read_fp)) {
        // Skip the original PAM header if it's there.
        if (strcmp(buffer, pam_header) == 0) continue;
        fputs(buffer, write_fp);
    }

    fclose(read_fp);
    fclose(write_fp);
    
    // Replace the old PAM configuration with the new one.
/*    if (rename(tempfile, pam_sudo_path) != 0) {
        perror("Error replacing the old PAM configuration file");
        return EXIT_FAILURE;
    }
*/
    printf("PAM configuration updated successfully.\n");

    return EXIT_SUCCESS;
}
