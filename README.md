# [NFF] Non-Followers Finder (aka unfollowers)
A simple script that just uses inforamtion that instagram provides, compares 2 lists, and outputs a list of people who you follow, but dont follow you and checks for deativated accounts if you want.

## Features
* Single executable file.
* All processing happens locally on your computer.
* Automatically unzips your data, locates the correct HTML files, and extracts the usernames.
* Generates an output HTML in a similar style to the original
* Includes an optional, rate-limited mode to detect and remove deleted/deactivated accounts from your final list using your browser session cookie.
* 
## How to Use

### Step 1: Request Your Instagram Data
To use this tool, you need to request a backup of your data directly from Instagram.

1. Open the Instagram app or website and go to **Settings and activity**.
2. Go to **Accounts Center** -> **Your information and permissions** -> **Download your information**.
3. Click **Download or transfer information**.
5. Scroll down and make sure the box for **Followers and following** is checked, under the **Connections** category
   (You can download other info too, the scipt only needs **Followers and following** to be selected).
8. Select the Format to **HTML**. Change the Date range to **All time**.
9. Click **Create files**. 
10. Instagram will email you a `.zip` file when it's ready (usually within a few minutes).

### Step 2: Run the Tool
1. Download the .exe.
2. Drag and drop the `.zip` file you downloaded from Instagram's email **directly onto the `.exe` file**.
   *(Alternatively, run it from the command line: `*script_name.exe* *path/to/your/file.zip*`)*
3. The tool will automatically extract the files, compare your followers and following lists, and identify the non-followers.

### Step 3: (Optional) The Deactivated Account Check
Once the initial list is generated, the tool will ask if you want to run a "Deactivated Account Check." 

Instagram leaves deleted or banned accounts in your "Following" list. To filter these out, the tool needs a temporary session cookie to ping the Instagram servers. (YOU CAN ALSO USE IT AS A GUEST (LEAVE COOKIE BLANK), BUT IT MAY LIMIT YOU FAST AND I DONT RECCOMEND IT)

**If you choose YES:**
1. Open your browser and log into Instagram.
2. Press `F12` to open Developer Tools.
3. Go to the **Application** tab (or **Storage** in Firefox).
4. On the left sidebar, expand **Cookies** and click on `https://www.instagram.com`.
5. Find the row named `sessionid`. Copy the long string of text in the Value column.
6. Paste it into the command prompt and hit Enter.

## Output
The tool will generate a file named `non_followers.html` in the same directory. Open this file in any web browser to see a clean, Instagram-styled list of everyone who doesn't follow you back. You can middle-click (open in new tab) the links to quickly review and unfollow them.
e source code: `gcc main.c -o Instagram_Deadbeat_Checker.exe`

## Disclaimer
The script tries to respects Instagram's API rate limits. If your list is long, it will automatically pause periodically to keep your account safe from temporary blocks. 
This project is not affiliated with, endorsed by, or sponsored by Instagram or Meta Platforms, Inc. This is a local data-parsing tool meant for personal account management. Use the optional active-account checking feature responsibly.

**I AM NOT RESPONSIBLE FOR ANY ACCOUNT BANS, RESTRICTIONS, OR ACTION BLOCKS. WHILE HIGHLY UNLIKELY IF YOU FOLLOW THE INSTRUCTIONS, AUTOMATED SCRAPING ALWAYS CARRIES SOME RISK. I DO NOT GUARANTEE THAT YOUR ACCOUNT WON'T BE BANNED. USE AT YOUR OWN RISK.**
