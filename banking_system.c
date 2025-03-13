#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_USERS 100
#define MAX_TRANS 100
#define FILENAME "users.dat"

typedef struct {
    char type[20];
    float amount;
    char target[50];
    time_t timestamp;
} Transaction;

typedef struct {
    char username[50];
    char password[50];
    float balance;
    Transaction transactions[MAX_TRANS];
    int transaction_count;
    BOOL isAdmin;
} User;

User users[MAX_USERS];
int userCount = 0;
HWND hUsername, hPassword, hRegUser, hRegPass;
User* loggedInUser = NULL;

// File operations
void saveUsers() {
    FILE* file = fopen(FILENAME, "wb");
    if (file) {
        fwrite(users, sizeof(User), userCount, file);
        fclose(file);
    }
}

void loadUsers() {
    FILE* file = fopen(FILENAME, "rb");
    if (file) {
        userCount = fread(users, sizeof(User), MAX_USERS, file);
        fclose(file);
    }
    
    if (userCount == 0) {
        strncpy(users[0].username, "admin", 50);
        strncpy(users[0].password, "admin123", 50);
        users[0].balance = 10000.0f;
        users[0].transaction_count = 0;
        users[0].isAdmin = TRUE;
        userCount = 1;
        saveUsers();
    }
}

// Transaction management
void addTransaction(User* user, const char* type, float amount, const char* target) {
    if (user->transaction_count < MAX_TRANS) {
        Transaction* t = &user->transactions[user->transaction_count++];
        strncpy(t->type, type, 20);
        t->amount = amount;
        t->timestamp = time(NULL);
        if (target) strncpy(t->target, target, 50);
    }
}

// Balance operations
BOOL deposit(User* user, float amount) {
    if (amount <= 0) return FALSE;
    user->balance += amount;
    addTransaction(user, "Deposit", amount, NULL);
    return TRUE;
}

BOOL withdraw(User* user, float amount) {
    if (amount <= 0 || amount > user->balance) return FALSE;
    user->balance -= amount;
    addTransaction(user, "Withdraw", -amount, NULL);
    return TRUE;
}

BOOL transfer(User* sender, const char* receiverName, float amount) {
    if (amount <= 0 || amount > sender->balance) return FALSE;
    
    User* receiver = NULL;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, receiverName) == 0) {
            receiver = &users[i];
            break;
        }
    }
    if (!receiver) return FALSE;
    
    sender->balance -= amount;
    receiver->balance += amount;
    addTransaction(sender, "Transfer", -amount, receiverName);
    addTransaction(receiver, "Transfer", amount, sender->username);
    return TRUE;
}

// Main window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hUserLabel, hPassLabel, hLoginBtn, hRegBtn;
    static HWND hRegLabel, hRegUserLabel, hRegPassLabel, hRegSubmit;
    
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
            
            // Login UI
            hUserLabel = CreateWindow("STATIC", "Username:", WS_VISIBLE | WS_CHILD,
                20, 20, 80, 25, hwnd, NULL, NULL, NULL);
            hUsername = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER,
                110, 20, 150, 25, hwnd, NULL, NULL, NULL);
            
            hPassLabel = CreateWindow("STATIC", "Password:", WS_VISIBLE | WS_CHILD,
                20, 60, 80, 25, hwnd, NULL, NULL, NULL);
            hPassword = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD,
                110, 60, 150, 25, hwnd, NULL, NULL, NULL);
            
            hLoginBtn = CreateWindow("BUTTON", "Login", WS_VISIBLE | WS_CHILD,
                110, 100, 80, 30, hwnd, (HMENU)1, NULL, NULL);
            hRegBtn = CreateWindow("BUTTON", "Register", WS_VISIBLE | WS_CHILD,
                110, 140, 80, 30, hwnd, (HMENU)2, NULL, NULL);
            
            // Registration UI (hidden initially)
            hRegLabel = CreateWindow("STATIC", "Registration", WS_CHILD,
                20, 20, 100, 25, hwnd, NULL, NULL, NULL);
            hRegUserLabel = CreateWindow("STATIC", "Username:", WS_CHILD,
                20, 50, 80, 25, hwnd, NULL, NULL, NULL);
            hRegUser = CreateWindow("EDIT", "", WS_CHILD | WS_BORDER,
                110, 50, 150, 25, hwnd, NULL, NULL, NULL);
            hRegPassLabel = CreateWindow("STATIC", "Password:", WS_CHILD,
                20, 80, 80, 25, hwnd, NULL, NULL, NULL);
            hRegPass = CreateWindow("EDIT", "", WS_CHILD | WS_BORDER | ES_PASSWORD,
                110, 80, 150, 25, hwnd, NULL, NULL, NULL);
            hRegSubmit = CreateWindow("BUTTON", "Submit", WS_CHILD,
                110, 120, 80, 30, hwnd, (HMENU)3, NULL, NULL);
            
            // Set fonts
            SendMessage(hUsername, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hPassword, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hRegUser, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hRegPass, WM_SETFONT, (WPARAM)hFont, TRUE);
            return 0;
        }
        
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) { // Login
                char user[50] = {0}, pass[50] = {0};
                GetWindowText(hUsername, user, 50);
                GetWindowText(hPassword, pass, 50);
                
                BOOL found = FALSE;
                for (int i = 0; i < userCount; i++) {
                    if (strcmp(users[i].username, user) == 0 && 
                        strcmp(users[i].password, pass) == 0) {
                        loggedInUser = &users[i];
                        found = TRUE;
                        break;
                    }
                }

                if (found) {
                    // Clear and hide login UI
                    SetWindowText(hUsername, "");
                    SetWindowText(hPassword, "");
                    ShowWindow(hUserLabel, SW_HIDE);
                    ShowWindow(hUsername, SW_HIDE);
                    ShowWindow(hPassLabel, SW_HIDE);
                    ShowWindow(hPassword, SW_HIDE);
                    ShowWindow(hLoginBtn, SW_HIDE);
                    ShowWindow(hRegBtn, SW_HIDE);

                    // Create dashboard
                    if (loggedInUser->isAdmin) {
                        CreateWindow("BUTTON", "Manage Users", WS_VISIBLE | WS_CHILD,
                            50, 20, 120, 30, hwnd, (HMENU)10, NULL, NULL);
                    } else {
                        CreateWindow("BUTTON", "Deposit", WS_VISIBLE | WS_CHILD,
                            50, 20, 80, 30, hwnd, (HMENU)4, NULL, NULL);
                        CreateWindow("BUTTON", "Withdraw", WS_VISIBLE | WS_CHILD,
                            50, 60, 80, 30, hwnd, (HMENU)5, NULL, NULL);
                        CreateWindow("BUTTON", "Transfer", WS_VISIBLE | WS_CHILD,
                            50, 100, 80, 30, hwnd, (HMENU)6, NULL, NULL);
                    }
                    CreateWindow("BUTTON", "Logout", WS_VISIBLE | WS_CHILD,
                        150, 180, 80, 30, hwnd, (HMENU)7, NULL, NULL);
                } else {
                    MessageBox(hwnd, "Invalid credentials!", "Error", MB_ICONERROR);
                }
            }
            if (LOWORD(wParam) == 2) { // Show registration
                ShowWindow(hRegLabel, SW_SHOW);
                ShowWindow(hRegUserLabel, SW_SHOW);
                ShowWindow(hRegUser, SW_SHOW);
                ShowWindow(hRegPassLabel, SW_SHOW);
                ShowWindow(hRegPass, SW_SHOW);
                ShowWindow(hRegSubmit, SW_SHOW);
                // Hide login UI
                ShowWindow(hUserLabel, SW_HIDE);
                ShowWindow(hUsername, SW_HIDE);
                ShowWindow(hPassLabel, SW_HIDE);
                ShowWindow(hPassword, SW_HIDE);
                ShowWindow(hLoginBtn, SW_HIDE);
                ShowWindow(hRegBtn, SW_HIDE);
            }
            if (LOWORD(wParam) == 3) { // Submit registration
                char user[50] = {0}, pass[50] = {0};
                GetWindowText(hRegUser, user, 50);
                GetWindowText(hRegPass, pass, 50);
                
                if (strlen(user) > 0 && strlen(pass) > 0) {
                    strncpy(users[userCount].username, user, 50);
                    strncpy(users[userCount].password, pass, 50);
                    users[userCount].balance = 0.0f;
                    users[userCount].isAdmin = FALSE;
                    users[userCount].transaction_count = 0;
                    userCount++;
                    saveUsers();
                    
                    // Clear and show login
                    SetWindowText(hRegUser, "");
                    SetWindowText(hRegPass, "");
                    ShowWindow(hRegLabel, SW_HIDE);
                    ShowWindow(hRegUserLabel, SW_HIDE);
                    ShowWindow(hRegUser, SW_HIDE);
                    ShowWindow(hRegPassLabel, SW_HIDE);
                    ShowWindow(hRegPass, SW_HIDE);
                    ShowWindow(hRegSubmit, SW_HIDE);
                    ShowWindow(hUserLabel, SW_SHOW);
                    ShowWindow(hUsername, SW_SHOW);
                    ShowWindow(hPassLabel, SW_SHOW);
                    ShowWindow(hPassword, SW_SHOW);
                    ShowWindow(hLoginBtn, SW_SHOW);
                    ShowWindow(hRegBtn, SW_SHOW);
                    
                    MessageBox(hwnd, "Registration successful!", "Success", MB_OK);
                } else {
                    MessageBox(hwnd, "Please fill all fields!", "Error", MB_ICONERROR);
                }
            }
            if (LOWORD(wParam) == 7) { // Logout
                loggedInUser = NULL;
                // Show login UI
                ShowWindow(hUserLabel, SW_SHOW);
                ShowWindow(hUsername, SW_SHOW);
                ShowWindow(hPassLabel, SW_SHOW);
                ShowWindow(hPassword, SW_SHOW);
                ShowWindow(hLoginBtn, SW_SHOW);
                ShowWindow(hRegBtn, SW_SHOW);
                // Destroy dashboard buttons
                for (int i = 4; i <= 10; i++) DestroyWindow(GetDlgItem(hwnd, i));
            }
            return 0;
        }
        
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int showCmd) {
    loadUsers();
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = "BankSystem";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindow("BankSystem", "Banking System", WS_OVERLAPPEDWINDOW,
        100, 100, 400, 300, NULL, NULL, hInst, NULL);
        
    ShowWindow(hwnd, showCmd);
    UpdateWindow(hwnd);
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}




hamim
