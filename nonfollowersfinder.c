#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <ctype.h>
#include <time.h>
#define DELAY_TIME 2000 // how long it waits before every account check

int extractor(FILE *file, const char *search_str, char ***arr_out){
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(fsize + 1);
    if(!buffer){return 0;}
    
    size_t read_size = fread(buffer, 1, fsize, file);
    buffer[read_size] = '\0';

    int count = 0;
    int capacity = 100;
    char **arr = malloc(capacity * sizeof(char*));

    char *ptr = buffer;
    char *end_ptr;

    while ((ptr = strstr(ptr, search_str)) != NULL) {
        ptr += strlen(search_str);
        end_ptr = strchr(ptr, '"');
        
        if (end_ptr) {
            int len = end_ptr - ptr;
            
            if (count >= capacity) {
                capacity *= 2;
                arr = realloc(arr, capacity * sizeof(char*));
            }
            
            arr[count] = malloc(len + 1);
            strncpy(arr[count], ptr, len);
            arr[count][len] = '\0';
            
            count++;
            ptr = end_ptr;
        }
    }
    
    free(buffer);
    *arr_out = arr;
    
    return count;
}
int is_account_active(const char *username, const char *sessionid){
    char command[1024];
    const char *temp_file = "temp_ig_check.html";

    // Your exact curl command
    sprintf(command,
        "curl -s -L --max-time 10 -o \"%s\" "
        "-H \"Cookie: sessionid=%s\" "
        "-A \"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36\" "
        "\"https://www.instagram.com/%s/\"",
        temp_file, sessionid, username);
    system(command);

    FILE *f = fopen(temp_file, "r");
    if (!f) return 1; // Failsafe: assume active if file read fails

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *content = malloc(size + 1);
    size_t got = fread(content, 1, size, f);
    content[got] = '\0';
    fclose(f);
    
    // Clean up the temp file
    remove(temp_file);

    // THE FIX: Check for the generic title tag or a 404 indicator
    // If it's just <title>Instagram</title>, the profile data didn't load (dead or blocked)
    int dead = (strstr(content, "<title>Instagram</title>") != NULL ||
                strstr(content, "Page Not Found") != NULL);

    free(content);
    
    // If dead == 1, return 0 (false/inactive). If dead == 0, return 1 (true/active).
    return !dead; 
}
int is_cookie_valid(const char *sessionid){
    char command[1024];
    const char *temp_file = "temp_cookie_check.html";

    printf("Checking if session cookie is valid...\n");

    // testing the cookie by trying to visit @instagram
    sprintf(command,
        "curl -s -L --max-time 10 -o \"%s\" "
        "-H \"Cookie: sessionid=%s\" "
        "-A \"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36\" "
        "\"https://www.instagram.com/instagram/\"",
        temp_file, sessionid);
    
    system(command);

    FILE *f = fopen(temp_file, "r");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (size <= 0) {
        fclose(f);
        remove(temp_file);
        return 0; // request failed or returned nothing, cookie is NOT valid
    }

    char *content = malloc(size + 1);
    if (!content) {
        fclose(f);
        remove(temp_file);
        return 0;
    }
    size_t got = fread(content, 1, size, f);
    content[got] = '\0';
    fclose(f);
    remove(temp_file);

    // check html
    int is_valid = (strstr(content, "<title>Instagram</title>") == NULL);
    free(content);
    return is_valid;
}

int main(int argc, char *argv[]) {
    // opening ZIP and finding files
    if(argc < 2){
        printf("Please drag and drop a ZIP file onto this executable.\n");
        system("pause");
        return 1;
    }

    const char *zip_path = argv[1];
    const char *internal_html_path = "/connections/followers_and_following";
    const char *temp_dir = "temp_unzip_folder";
    char *following_html = "following.html";
    char *followers_html = "followers_1.html";

    char command[1024];
    printf("Extracting ZIP...\n");
    sprintf(command, "mkdir %s 2>nul", temp_dir); 
    system(command);
    sprintf(command, "tar -xf \"%s\" -C %s", zip_path, temp_dir);
    if(system(command) != 0){
        printf("Failed to extract the zip file.\n"); 
        system("pause");
        return 1;
    }

    // paths creation and file reading
    char full_following_path[1024];
    char full_followers_path[1024];
    sprintf(full_following_path, "%s/%s/%s", temp_dir, internal_html_path, following_html);
    sprintf(full_followers_path, "%s/%s/%s", temp_dir, internal_html_path, followers_html);
    FILE *file_following = fopen(full_following_path, "r");
    FILE *file_followers = fopen(full_followers_path, "r");
    if(file_following == NULL || file_followers == NULL){
        printf("Could not find the HTML files after extracting.\n");
        system("pause");
        return 1;
    }

    // --- actual code ---

    // extraction
    char **arr_following = NULL;
    char **arr_followers = NULL;
    int N = extractor(file_following, "href=\"https://www.instagram.com/_u/", &arr_following);
    fclose(file_following);
    printf("\n--- %d Following extracted ---\n", N);
    int M = extractor(file_followers, "href=\"https://www.instagram.com/", &arr_followers);
    fclose(file_followers);
    printf("--- %d Followers extracted ---\n", M);

    // what is contained in FOLLOWING, but NOT in FOLLWERS
    int NM = 0;
    char **arr_output = malloc(N * sizeof(char*)); 
    
    for(int i = 0; i < N; i++){
        int follows_back = 0;
        for(int j = 0; j < M; j++){
            if(strcmp(arr_following[i], arr_followers[j]) == 0){
                follows_back = 1;
                break;
            }
        }
        
        if(follows_back == 0){
            arr_output[NM] = arr_following[i];
            NM++;
        }
    }
    
    arr_output = realloc(arr_output, NM * sizeof(char*));

    printf("\n--- %d Non-followers found ---\n", NM);
    /*
    for(int i = 0; i < NM; i++){
        printf("  %d: %s\n", i + 1, arr_output[i]);
    }
    */ //no need to print

    // file generation
    FILE *out_file = fopen("non_followers.html", "w");
    if(out_file){
        //yapping here
        fprintf(out_file, "<!DOCTYPE html>\n<html>\n<head>\n<title>Non-Followers</title>\n<style>\nbody { font-family: Helvetica, Arial, sans-serif; background-color: #f0f2f5; color: #1c1e21; padding: 20px; }\n.container { max-width: 600px; margin: 0 auto; }\n.header { background: #fff; border-radius: 8px; padding: 15px; margin-bottom: 20px; box-shadow: 0 1px 2px rgba(0,0,0,0.1); }\n.card { background: #fff; border-radius: 8px; padding: 15px; margin-bottom: 8px; box-shadow: 0 1px 2px rgba(0,0,0,0.1); }\na { color: #385898; text-decoration: none; font-weight: 600; font-size: 14px; display: block; }\na:hover { text-decoration: underline; }\n</style>\n</head>\n<body>\n<div class='container'>\n<div class='header'>\n<h2 style='margin: 0 0 5px 0; font-size: 16px;'>Non-Followers (%d)</h2>\n<p style='margin: 0; color: #606770; font-size: 12px;'>Profiles you follow that don't follow you back</p>\n</div>\n", NM);
        
        for(int i = 0; i < NM; i++){
            fprintf(out_file, "<div class='card'>\n");
            fprintf(out_file, "<a href='https://www.instagram.com/%s' target='_blank'>%s</a>\n", arr_output[i], arr_output[i]);
            fprintf(out_file, "</div>\n");
        }
        fprintf(out_file, "</div>\n</body>\n</html>\n");
        fclose(out_file);
        printf("\nHTML Generated.\n");
    }else{
        printf("\nError: Could not create the HTML file.\n");
    }

    //  --- deactivated acc check (gets messy here...) ---

    char choice[8];
    printf("\nWould you like a deactivated accounts check?\n");
    printf(" [!] Instagram limits checks to ~1000 per day and ~200 per hour.\n [!] The script has pauses but instagram may suspect your account if you do too many checks.\n [!] You will need to provide a session cookie for these checks. (not stored)\n [!] %d non-followers found (includes non-active accoutns).", NM);
    printf("\nDo you want to proceed? (it may take a while) [y/n]: ");
    fgets(choice, sizeof(choice), stdin);

    if(choice[0] == 'y' || choice[0] == 'Y'){
        // check for large lists
        if(NM > 1000) {
            printf("\n [!] You have more than 1000 non-followers.\n", NM);
            printf("Checking this many accounts will take a very long time.\nInstagram may suspect your account. \nCONTINUE AT YOUR OWN RISK.\n");
            printf("Do you really want to proceed? [y/n]: ");
            char proceed[8];
            fgets(proceed, sizeof(proceed), stdin);
            if(proceed[0] != 'y' && proceed[0] != 'Y') {
                printf("Skipping deactivated check.\n");
                goto skip_deactivated_check;
            }
        }

        //cookie stuff
        char cookie[512];
        int cookie_is_valid = 0;
        while (1) {
            printf("\nPaste your Instagram 'sessionid' cookie and press Enter.\n");
            printf("(DevTools [F12] -> Application -> Cookies -> https://www.instagram.com -> sessionid (Value))\n> ");
            printf("[!] NOTE: If you only have a few accounts to check, you can skip this \n");
            printf("          and check as a guest by just pressing Enter with no cookie,\n");
            printf("          but you will get rate limited quickly.\n");
            printf("> ");
            fgets(cookie, sizeof(cookie), stdin);
            cookie[strcspn(cookie, "\n")] = '\0';

            if (!is_cookie_valid(cookie)) {
                printf("\n [ERROR] Your session cookie is invalid or expired! Try again.\n");
                continue;
            }
            
            printf("\n [SUCCESS] Cookie is valid! Proceeding...\n");
            cookie_is_valid = 1;
            break;
        }
        if(cookie_is_valid){
            printf("\nChecking for deactivated accounts...\n\n");
            char **arr_active = malloc(NM * sizeof(char*));
            int active_count = 0;
            int checks_this_hour = 0;
            int checks_total = 0;
            
            srand(time(NULL)); // seed for random num gen

            for(int i = 0; i < NM; i++){
                printf(" [%d/%d] Checking @%s ... ", i + 1, NM, arr_output[i]);
                fflush(stdout);
                
                if(is_account_active(arr_output[i], cookie)){
                    printf("active\n");
                    arr_active[active_count++] = arr_output[i];
                } else {
                    printf("DEACTIVATED\n");
                }

                // --- rate limits ---

                checks_this_hour++;
                checks_total++;
                // daily limit check (950)
                if (checks_total >= 950) {
                    printf("\n [RATE LIMIT] Hit the daily safety wall (950). Stopping checks to prevent ban...\n\n");
                    break;
                } 
                // hourly limit check (180)
                else if (checks_this_hour >= 180) {
                    printf("\n [RATE LIMIT] Hit the hourly safety wall (180). Cooling down for 15 minutes...\n\n");
                    Sleep(900000);
                    checks_this_hour = 0;
                } 
                // standard delay
                else if (i < NM - 1) {
                    int delay = DELAY_TIME + (rand() % 2001);
                    Sleep(delay);
                }
            }

            printf("\n--- %d Non-followers remaining ---\n", active_count);

            // update the html
            FILE *out_file2 = fopen("non_followers.html", "w");
            if(out_file2){
                fprintf(out_file2,
                    "<!DOCTYPE html>\n<html>\n<head>\n<title>Non-Followers (Checked)</title>\n<style>\nbody { font-family: Helvetica, Arial, sans-serif; background-color: #f0f2f5; color: #1c1e21; padding: 20px; }\n.container { max-width: 600px; margin: 0 auto; }\n.header { background: #fff; border-radius: 8px; padding: 15px; margin-bottom: 20px; box-shadow: 0 1px 2px rgba(0,0,0,0.1); }\n.card { background: #fff; border-radius: 8px; padding: 15px; margin-bottom: 8px; box-shadow: 0 1px 2px rgba(0,0,0,0.1); }\na { color: #385898; text-decoration: none; font-weight: 600; font-size: 14px; display: block; }\na:hover { text-decoration: underline; }\n</style>\n</head>\n<body>\n<div class='container'>\n<div class='header'>\n<h2 style='margin: 0 0 5px 0; font-size: 16px;'>Non-Followers (%d)</h2>\n<p style='margin: 0; color: #606770; font-size: 12px;'>Profiles you follow that don't follow you back</p>\n</div>\n", active_count);
                for(int i = 0; i < active_count; i++){
                    fprintf(out_file2, "<div class='card'>\n<a href='https://www.instagram.com/%s' target='_blank'>%s</a>\n</div>\n", arr_active[i], arr_active[i]);
                }
                fprintf(out_file2, "</div>\n</body>\n</html>\n");
                fclose(out_file2);
                printf("HTML updated.\n");
            }
            free(arr_active);
        }
    }
    
    skip_deactivated_check: // goto if they cancel check
    system("pause");

    // clean up
    sprintf(command, "rmdir /s /q %s", temp_dir);
    system(command);
    for(int i = 0; i < N; i++) free(arr_following[i]);
    free(arr_following);
    for(int i = 0; i < M; i++) free(arr_followers[i]);
    free(arr_followers);
    free(arr_output);
    return 0;
}
