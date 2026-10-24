🔐 Password Strength & Pattern Analyser

A C++ command-line application that evaluates password strength using character diversity, entropy estimation, and common password pattern detection. It provides a strength score, estimated cracking time, and suggestions to improve password security.

✨ Features
Password Strength Scoring: Rates passwords from Very Weak to Very Strong on a scale of 0–100.
Entropy Calculation: Estimates raw and effective password entropy in bits.
Character Analysis: Detects lowercase letters, uppercase letters, digits, and symbols.
Pattern Detection:
Repeated characters (e.g., aaa, 1111)
Repeated blocks (e.g., abcabc)
Ascending and descending sequences (e.g., abcd, 4321)
Keyboard patterns (e.g., qwerty, asdf)
Common passwords and dictionary words
Leetspeak variations (e.g., p@ssw0rd)
Year-like numbers (e.g., 2024)
Passwords with low character variety
Predictable word-and-digit combinations
Cracking Time Estimation: Estimates offline guessing time using an assumed rate of 10 billion guesses per second.
Improvement Suggestions: Provides recommendations based on detected weaknesses.
Batch Analysis: Analyses multiple passwords from a text file.
Privacy-Focused: Runs locally and does not transmit passwords over the network.
🛠️ Technologies Used
C++
Standard Template Library (STL)
File handling (fstream)
String processing and algorithms
Mathematical functions (cmath)
📋 Requirements
A C++ compiler supporting C++11 or later, such as GCC or Clang.
A terminal or command prompt.
🚀 Installation and Usage
1. Clone the Repository
git clone https://github.com/YOUR-USERNAME/YOUR-REPOSITORY.git


Replace YOUR-USERNAME and YOUR-REPOSITORY with your GitHub username and repository name.

2. Navigate to the Project Directory
cd YOUR-REPOSITORY

3. Compile the Program

Save the source code as password_analyser.cpp, then run:

g++ -std=c++11 password_analyser.cpp -o password_analyser

4. Run the Program

Windows:

password_analyser.exe


Linux / macOS:

./password_analyser

💻 How to Use

After starting the program, select one of the following options:

Analyse a password — Enter a password to view its strength report, detected patterns, entropy, estimated cracking time, and suggestions.
Analyse passwords from a file — Provide the path to a text file containing one password per line.
Exit — Close the application.
Example Input
Enter password: password123
Show password in report? (y/n): n

Example Output

The exact score and detected patterns depend on the password entered.

==================== REPORT ====================
Password : ***********
Length   : 11
Classes  : lower digits
Entropy  : [calculated entropy]
Strength : [calculated rating] [strength bar]
Crack est: [estimated cracking time]

Patterns detected:
  - Contains common word "password"

Suggestions:
  - Use at least 12 characters (longer is better).
  - Add uppercase letters.
  - Add symbols such as ! @ # $ %.
  - Avoid the detected patterns.
================================================


Note: The example is illustrative; actual values are calculated by the program.

📂 Batch Password Analysis

Create a text file named passwords.txt with one password per line:

password123
qwerty
Summer2024
MyRandomPhrase!82


Select option 2 and enter the file name:

passwords.txt


The program displays each password's rating, score, and number of detected patterns, along with the average score.

Security note: Batch mode displays passwords in its output. Use test passwords rather than real credentials.

🧠 How It Works

The analyser performs the following steps:

Determines password length and character classes.
Calculates raw entropy based on password length and the estimated character pool.
Detects predictable patterns and assigns heuristic penalties.
Calculates effective entropy after subtracting penalties.
Maps effective entropy to a strength score from 0 to 100.
Generates a strength rating and improvement suggestions.
Estimates cracking time using the effective entropy and the configured guessing rate.
📊 Password Strength Ratings
Score	Rating
0–19	Very Weak
20–39	Weak
40–59	Fair
60–79	Strong
80–100	Very Strong

Passwords shorter than 8 characters are capped at a score of 25.

⚠️ Limitations
Entropy and cracking-time values are estimates, not guarantees.
The assumed guessing rate of 10 billion guesses per second may not reflect real attack conditions.
Pattern penalties are heuristic and are not a substitute for a dedicated password-strength estimation library.
The common-password list is limited to predefined entries.
The program is an educational security project, not a password-storage or authentication system.

Important: Do not use this tool as the sole measure of whether a password is secure. For real-world password storage, use a dedicated password-hashing algorithm such as Argon2id, scrypt, or bcrypt.

🔒 Privacy

The program analyses input locally and contains no network-transmission functionality. However, avoid entering real passwords into tools that display, log, or otherwise expose them.

🎯 Project Objective

The objective of this project is to demonstrate C++ programming concepts, including string manipulation, functions, vectors, sets, file handling, entropy calculations, and heuristic pattern detection in a practical cybersecurity application.

👨‍💻 Author

Your Name

GitHub: @YOUR-USERNAME
📄 License

This project is available for educational and learning purposes. Add a license file if you wish to specify formal reuse and distribution terms.
