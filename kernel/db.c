#include "kernel.h"

#define MAX_TABLES 8
#define MAX_ROWS 64
#define MAX_COLS 16
#define MAX_COL_NAME 32
#define MAX_DATA 256
#define MAX_INDEXES 8
#define MAX_TRANSACTIONS 4
#define MAX_LOG_SIZE 1024

typedef struct {
    char name[MAX_COL_NAME];
    int type;  // 0 = string, 1 = int
} column_t;

typedef struct {
    int row_idx;
    char key[MAX_DATA];
} index_entry_t;

typedef struct {
    char name[32];
    column_t columns[MAX_COLS];
    int col_count;
    char data[MAX_ROWS][MAX_COLS][MAX_DATA];
    int row_count;
    index_entry_t indexes[MAX_INDEXES];
    int index_count;
} table_t;

typedef struct {
    int active;
    char log[MAX_LOG_SIZE];
    int log_pos;
    int saved_row_count[MAX_TABLES];
} transaction_t;

static table_t tables[MAX_TABLES];
static int table_count = 0;
static transaction_t transactions[MAX_TRANSACTIONS];
static int current_transaction = -1;

void db_init() {
    for (int i = 0; i < MAX_TABLES; i++) {
        tables[i].name[0] = '\0';
        tables[i].col_count = 0;
        tables[i].row_count = 0;
        tables[i].index_count = 0;
    }
    table_count = 0;
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        transactions[i].active = 0;
        transactions[i].log_pos = 0;
    }
    current_transaction = -1;
}

void db_create_table(const char* name) {
    if (table_count >= MAX_TABLES) return;
    
    // Check if table exists
    for (int i = 0; i < table_count; i++) {
        int match = 1;
        for (int j = 0; name[j] != '\0' || tables[i].name[j] != '\0'; j++) {
            if (name[j] != tables[i].name[j]) {
                match = 0;
                break;
            }
        }
        if (match) return;  // Table exists
    }
    
    int idx = table_count++;
    for (int i = 0; name[i] != '\0' && i < 31; i++) {
        tables[idx].name[i] = name[i];
    }
    tables[idx].name[31] = '\0';
    tables[idx].col_count = 0;
    tables[idx].row_count = 0;
    tables[idx].index_count = 0;
}

int db_add_column(const char* table_name, const char* col_name, int type) {
    for (int i = 0; i < table_count; i++) {
        int match = 1;
        for (int j = 0; table_name[j] != '\0' || tables[i].name[j] != '\0'; j++) {
            if (table_name[j] != tables[i].name[j]) {
                match = 0;
                break;
            }
        }
        if (match) {
            if (tables[i].col_count >= MAX_COLS) return -1;
            int col_idx = tables[i].col_count++;
            for (int j = 0; col_name[j] != '\0' && j < MAX_COL_NAME - 1; j++) {
                tables[i].columns[col_idx].name[j] = col_name[j];
            }
            tables[i].columns[col_idx].name[MAX_COL_NAME - 1] = '\0';
            tables[i].columns[col_idx].type = type;
            return 0;
        }
    }
    return -1;
}

int db_create_index(const char* table_name, const char* col_name) {
    for (int i = 0; i < table_count; i++) {
        int match = 1;
        for (int j = 0; table_name[j] != '\0' || tables[i].name[j] != '\0'; j++) {
            if (table_name[j] != tables[i].name[j]) {
                match = 0;
                break;
            }
        }
        if (match) {
            if (tables[i].index_count >= MAX_INDEXES) return -1;
            
            // Find column index
            int col_idx = -1;
            for (int k = 0; k < tables[i].col_count; k++) {
                int col_match = 1;
                for (int l = 0; col_name[l] != '\0' || tables[i].columns[k].name[l] != '\0'; l++) {
                    if (col_name[l] != tables[i].columns[k].name[l]) {
                        col_match = 0;
                        break;
                    }
                }
                if (col_match) {
                    col_idx = k;
                    break;
                }
            }
            if (col_idx < 0) return -1;
            
            // Create index entries
            for (int r = 0; r < tables[i].row_count && tables[i].index_count < MAX_INDEXES; r++) {
                tables[i].indexes[tables[i].index_count].row_idx = r;
                for (int c = 0; c < MAX_DATA - 1 && tables[i].data[r][col_idx][c] != '\0'; c++) {
                    tables[i].indexes[tables[i].index_count].key[c] = tables[i].data[r][col_idx][c];
                }
                tables[i].indexes[tables[i].index_count].key[MAX_DATA - 1] = '\0';
                tables[i].index_count++;
            }
            return 0;
        }
    }
    return -1;
}

int db_insert(const char* table_name, const char* data) {
    for (int i = 0; i < table_count; i++) {
        int match = 1;
        for (int j = 0; table_name[j] != '\0' || tables[i].name[j] != '\0'; j++) {
            if (table_name[j] != tables[i].name[j]) {
                match = 0;
                break;
            }
        }
        if (match) {
            if (tables[i].row_count >= MAX_ROWS) return -1;
            
            // Simple CSV parsing
            int row = tables[i].row_count;
            int col = 0;
            int pos = 0;
            
            for (int k = 0; data[k] != '\0' && col < MAX_COLS; k++) {
                if (data[k] == ',') {
                    tables[i].data[row][col][pos] = '\0';
                    col++;
                    pos = 0;
                } else {
                    if (pos < MAX_DATA - 1) {
                        tables[i].data[row][col][pos++] = data[k];
                    }
                }
            }
            tables[i].data[row][col][pos] = '\0';
            
            tables[i].row_count++;
            return 0;
        }
    }
    return -1;
}

char* db_select(const char* table_name, const char* key) {
    for (int i = 0; i < table_count; i++) {
        int match = 1;
        for (int j = 0; table_name[j] != '\0' || tables[i].name[j] != '\0'; j++) {
            if (table_name[j] != tables[i].name[j]) {
                match = 0;
                break;
            }
        }
        if (match) {
            // Search using index if available
            if (tables[i].index_count > 0) {
                for (int j = 0; j < tables[i].index_count; j++) {
                    int key_match = 1;
                    for (int k = 0; key[k] != '\0' || tables[i].indexes[j].key[k] != '\0'; k++) {
                        if (key[k] != tables[i].indexes[j].key[k]) {
                            key_match = 0;
                            break;
                        }
                    }
                    if (key_match) {
                        return tables[i].data[tables[i].indexes[j].row_idx][0];
                    }
                }
            }
            
            // Fallback: search all rows
            for (int r = 0; r < tables[i].row_count; r++) {
                if (strcmp(tables[i].data[r][0], key) == 0) {
                    return tables[i].data[r][0];
                }
            }
            return "";
        }
    }
    return NULL;
}

// Transaction functions
int db_begin() {
    if (current_transaction >= 0) return -1;  // Already in transaction
    
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        if (!transactions[i].active) {
            current_transaction = i;
            transactions[i].active = 1;
            transactions[i].log_pos = 0;
            for (int j = 0; j < MAX_TABLES; j++) {
                transactions[i].saved_row_count[j] = tables[j].row_count;
            }
            return 0;
        }
    }
    return -1;
}

int db_commit() {
    if (current_transaction < 0) return -1;
    
    transactions[current_transaction].active = 0;
    current_transaction = -1;
    return 0;
}

int db_rollback() {
    if (current_transaction < 0) return -1;
    
    // Restore row counts
    for (int i = 0; i < table_count; i++) {
        tables[i].row_count = transactions[current_transaction].saved_row_count[i];
    }
    
    transactions[current_transaction].active = 0;
    current_transaction = -1;
    return 0;
}
