#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum {
    T_WORD,
    T_BYTE,
    T_REL,
};

enum {
    A_REG,
    A_LREG,
    A_NREG,
    A_OTHER,
};

struct label {
    char name[20];
    uint32_t org;
};

struct unres {
    char *filename;
    unsigned int line;
    char ex[35];
    uint32_t addr, org;
    char type;
};

unsigned char memory[16777316];

const char *regs[] = {
    "ZERO", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
    "R8", "R9", "R10", "R11", "R12", "R13", "RSP", "RPC",
};

const char *nregs[] = {
    "(ZERO)", "(R1)", "(R2)", "(R3)", "(R4)", "(R5)", "(R6)", "(R7)",
    "(R8)", "(R9)", "(R10)", "(R11)", "(R12)", "(R13)", "(RSP)", "(RPC)",
};

const char *lregs[] = {
    "LZERO", "L1", "L2", "L3", "L4", "L5", "L6", "L7",
    "L8", "L9", "L10", "L11", "L12", "L13", "LSP", "LPC",
};

unsigned int g_line;
char *g_filename = 0;

uint32_t addr = 0;
uint32_t org = 0;
uint32_t prev = 0;

struct unres unres[100];
int nunres = 0;
struct label labels[200];
int nlabels = 0;
int lastNonLocal = 0;

void error(const char *err) {
    printf("error at line %d in %s: %s\n", g_line, g_filename, err);
    exit(1);
}

char argType(char *s, int *np) {
    int i, n;
    char t;
    n = -1;
    t = A_OTHER;
    for(i = 0; i < 16; i++) {
        if(!strcmp(regs[i], s)) { n = i; t = A_REG; break; }
        if(!strcmp(lregs[i], s)) { n = i; t = A_LREG; break; }
        if(!strcmp(nregs[i], s)) { n = i; t = A_NREG; break; }
    }
    if(np) *np = n;
    return t;
}

void addByte(char b) {
    memory[addr++] = b;
    org++;
}

void addWord(uint32_t w) {
    *(uint32_t*)&memory[addr] = w;
    addr += 4;
    org += 4;
}

void setWord(uint32_t ad, uint32_t w) {
    *(uint32_t*)&memory[ad] = w;
}

void resAhead() {
    int i;
    for(i = 0; i < nunres; i++) {
        if(strcmp(unres[i].ex, "+")) continue;
        switch(unres[i].type) {
        case T_WORD:
            setWord(unres[i].addr, org);
            break;
        case T_BYTE:
        case T_REL:
            memory[unres[i].addr] = org-unres[i].org-1;
            break;
        }
        unres[i] = unres[--nunres];
        i--;
    }
}

struct label *findLabel(char *name) {
    int i;
    for(i = nlabels-1; i >= 0; i--)
        if(!strcmp(labels[i].name, name))
            return &labels[i];
    return 0;
}

int number(char *s, uint32_t *n) {
    char neg = 0;
    char base = 10;

    if(*s == '-') {
        if(*(++s) == 0) return 0;
        neg = 1;
    }
    if(*s == '$') {
        if(*(++s) == 0) return 0;
        base = 16;
    } else if(*s == '%') {
        if(*(++s) == 0) return 0;
        base = 2;
    }

    *n = 0;
    do {
        if(base <= 10) {
            if(*s >= '0' && *s <= '0'+base-1)
                *n = *n * base + *s - '0';
            else return 0;
        } else {
            if(*s >= 'A' && *s <= 'A'+base-11)
                *n = *n * base + *s - 'A' + 10;
            else if(*s >= '0' && *s <= '9')
                *n = *n * base + *s - '0';
            else return 0;
        }
    } while (*(++s));

    if(neg) *n *= -1;

    return 1;
}

int evalVal(char *val, uint32_t *n) {
    const char *charError = "invalid char";
    struct label *lbl;

    if(!number(val, n)) {
        lbl = findLabel(val);
        if(!lbl) {
            if(val[0] == '\'') {
                if(val[1] == 0) error(charError);
                if(val[2] != '\'') error(charError);
                *n = val[1];
            } else return 0;
        } else *n = lbl->org;
    }
    return 1;
}

int tryEval(char *ex, uint32_t *n) {
    char *vals[20];
    char ops[20];
    char *p, *b;
    int nvals = 0, nops = 0;
    const char *exError = "invalid expression";
    uint32_t m;
    int i;

    b = ex;
    for(p = ex; *p; p++) {
        if(strchr("+-", *p)) {
            if(p == ex && *p == '-') continue;
            if(p == b) error(exError);
            ops[nops++] = *p;
            *p = 0;
            vals[nvals++] = b;
            b = ++p;
        }
    }
    if(b == p) error(exError);
    vals[nvals++] = b;

    if(nops != nvals-1) error(exError);

    if(!evalVal(vals[0], n)) return 0;

    for(i = 0; i < nops; i++) {
        if(!evalVal(vals[i+1], &m)) return 0;
        switch(ops[i]) {
        case '+':
            *n += m;
            break;
        case '-':
            *n -= m;
            break;
        }
    }

    return 1;
}

void resolve() {
    int i;
    uint32_t n;

    for(i = 0; i < nunres; i++) {
        if(!strcmp(unres[i].ex, "+")) continue;
        if(!tryEval(unres[i].ex, &n)) continue;

        switch(unres[i].type) {
        case T_WORD:
            setWord(unres[i].addr, n);
            break;
        case T_BYTE:
            memory[unres[i].addr] = n;
            break;
        case T_REL:
            memory[unres[i].addr] = (char)(n-unres[i].org-1);
            break;
        }
        unres[i] = unres[--nunres];
        i--;
    }
}

void addVal(char *ex, char type) {
    if(!strcmp(ex, "-")) {
        switch(type) {
        case T_WORD:
            addWord(prev);
            break;
        case T_BYTE:
        case T_REL:
            addByte(prev-(org+1));
            break;
        }
    } else {
        unres[nunres].filename = g_filename;
        unres[nunres].line = g_line;
        strcpy(unres[nunres].ex, ex);
        unres[nunres].type = type;
        unres[nunres].addr = addr;
        unres[nunres].org = org;
        nunres++;
        switch(type) {
        case T_WORD:
            addWord(0);
            break;
        case T_BYTE:
        case T_REL:
            addByte(0);
            break;
        }
        resolve();
    }
}

void asmFile(char *filename);

void asmLine(char *line) {
    const char *syntaxError = "syntax error";
    const char *nargsError = "wrong number of args";
    const char *argsError = "wrong combination of args";
    const char *evalError = "failed to evaluate expression";
    const char *white = " \t,:";
    char *tokens[40];
    int ntokens = 0;
    char buf[200];
    char *p, *b, *s;
    char a1, a2;
    int i, j;
    uint32_t n;
    uint32_t r1, r2;
    uint32_t oa;

    oa = addr;

    /* tokenize */

    b = buf;
    p = buf;

    for(;;) {
        if(*line == '"') {
            if(b != p) error(syntaxError);
            s = strchr(line+1, '"');
            if(!s) error("unterminated quote");
            while(line < s) *(p++) = *(line++);
            *p = 0;
            tokens[ntokens++] = b;
            b = ++p;
        } else {
            if(*line >= 'a' && *line <= 'z') *line += 'A'-'a';

            if(!(*line) || strchr(white, *line)) {
                if(b != p) {
                    *p = 0;
                    tokens[ntokens++] = b;
                    b = ++p;
                }
                if(!(*line)) break;
            } else {
                *(p++) = *line;
            }
        }
        line++;
    }

    /* assemble */

    /*for(i = 0; i < ntokens; i++) printf("%s ", tokens[i]);
    printf("\n");*/

assemble:

    if(!strcmp(tokens[0], "ORG")) {
        if(ntokens != 2) error(nargsError);
        if(!tryEval(tokens[1], &org)) error(evalError);
    } else if(!strcmp(tokens[0], "INCLUDE")) {
        if(ntokens != 2) error(nargsError);
        if(tokens[1][0] != '"') error(argsError);
        asmFile(tokens[1]+1);
    } else if(!strcmp(tokens[0], "DB")) {
        if(ntokens == 1) error(nargsError);
        for(i = 1; i < ntokens; i++) {
            if(tokens[i][0] == '"') {
                for(j = 1; tokens[i][j]; j++)
                    addByte(tokens[i][j]);
            } else
                addVal(tokens[i], T_BYTE);
        }
    } else if(!strcmp(tokens[0], "DW")) {
        if(ntokens == 1) error(nargsError);
        for(i = 1; i < ntokens; i++)
            addVal(tokens[i], T_WORD);
    } else if(!strcmp(tokens[0], "INT")) {
        if(ntokens != 2) error(nargsError);
        addByte(0x00);
        addVal(tokens[1], T_BYTE);
    } else if(!strcmp(tokens[0], "CALL")) {
        if(ntokens != 2) error(nargsError);
        addByte(0x01);
        addVal(tokens[1], T_WORD);
    } else if(!strcmp(tokens[0], "BEQ")) {
        if(ntokens != 4) error(nargsError);
        addByte(0x02);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 != A_REG || a2 != A_REG) error(argsError);
        addByte((r1<<4)|r2);
        addVal(tokens[3], T_REL);
    } else if(!strcmp(tokens[0], "BNE")) {
        if(ntokens != 4) error(nargsError);
        addByte(0x03);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 != A_REG || a2 != A_REG) error(argsError);
        addByte((r1<<4)|r2);
        addVal(tokens[3], T_REL);
    } else if(!strcmp(tokens[0], "BGE")) {
        if(ntokens != 4) error(nargsError);
        addByte(0x04);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 != A_REG || a2 != A_REG) error(argsError);
        addByte((r1<<4)|r2);
        addVal(tokens[3], T_REL);
    } else if(!strcmp(tokens[0], "BLT")) {
        if(ntokens != 4) error(nargsError);
        addByte(0x05);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 != A_REG || a2 != A_REG) error(argsError);
        addByte((r1<<4)|r2);
        addVal(tokens[3], T_REL);
    } else if(!strcmp(tokens[0], "MOV")) {
        if(ntokens != 3) error(nargsError);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 == A_REG && a2 == A_OTHER) {
            addByte(0x10|r1);
            addVal(tokens[2], T_WORD);
        } else if(a1 == A_REG && a2 == A_NREG) {
            addByte(0x06);
            addByte((r1<<4)|r2);
        } else if(a1 == A_NREG && a2 == A_REG) {
            addByte(0x07);
            addByte((r1<<4)|r2);
        } else if(a1 == A_LREG && a2 == A_NREG) {
            addByte(0x08);
            addByte((r1<<4)|r2);
        } else if(a1 == A_NREG && a2 == A_LREG) {
            addByte(0x09);
            addByte((r1<<4)|r2);
        } else if(a1 == A_REG && a2 == A_REG) {
            addByte(0x0A);
            addByte((r1<<4)|r2);
        } else error(argsError);
    } else if(!strcmp(tokens[0], "AND")) {
        if(ntokens != 3) error(nargsError);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 != A_REG || a2 != A_REG) error(argsError);
        addByte(0x0B);
        addByte((r1<<4)|r2);
    } else if(!strcmp(tokens[0], "OR")) {
        if(ntokens != 3) error(nargsError);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 != A_REG || a2 != A_REG) error(argsError);
        addByte(0x0C);
        addByte((r1<<4)|r2);
    } else if(!strcmp(tokens[0], "XOR")) {
        if(ntokens != 3) error(nargsError);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 != A_REG || a2 != A_REG) error(argsError);
        addByte(0x0D);
        addByte((r1<<4)|r2);
    } else if(!strcmp(tokens[0], "ADD")) {
        if(ntokens != 3) error(nargsError);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 == A_REG && a2 == A_OTHER) {
            addByte(0x20|r1);
            addVal(tokens[2], T_BYTE);
        } else if(a1 == A_REG && a2 == A_REG) {
            addByte(0x0E);
            addByte((r1<<4)|r2);
        } else error(argsError);
    } else if(!strcmp(tokens[0], "SUB")) {
        if(ntokens != 3) error(nargsError);
        a1 = argType(tokens[1], &r1);
        a2 = argType(tokens[2], &r2);
        if(a1 != A_REG || a2 != A_REG) error(argsError);
        addByte(0x0F);
        addByte((r1<<4)|r2);
    } else if(!strcmp(tokens[0], "PUSH")) {
        if(ntokens != 2) error(nargsError);
        a1 = argType(tokens[1], &r1);
        if(a1 != A_REG) error(argsError);
        addByte(0x30|r1);
    } else if(!strcmp(tokens[0], "POP")) {
        if(ntokens != 2) error(nargsError);
        a1 = argType(tokens[1], &r1);
        if(a1 != A_REG) error(argsError);
        addByte(0x40|r1);
    } else if(!strcmp(tokens[0], "INV")) {
        if(ntokens != 2) error(nargsError);
        a1 = argType(tokens[1], &r1);
        if(a1 != A_REG) error(argsError);
        addByte(0x50|r1);
    } else if(!strcmp(tokens[0], "SHR")) {
        if(ntokens != 3) error(nargsError);
        a1 = argType(tokens[1], &r1);
        if(a1 != A_REG) error(argsError);
        addByte(0x60|r1);
        addVal(tokens[2], T_BYTE);
    } else if(!strcmp(tokens[0], "SHL")) {
        if(ntokens != 3) error(nargsError);
        a1 = argType(tokens[1], &r1);
        if(a1 != A_REG) error(argsError);
        addByte(0x70|r1);
        addVal(tokens[2], T_BYTE);
    } else if(!strcmp(tokens[0], "BRA")) {
        if(ntokens != 2) error(nargsError);
        addByte(0x2F);
        addVal(tokens[1], T_REL);
    } else if(!strcmp(tokens[0], "RET")) {
        if(ntokens != 1) error(nargsError);
        addByte(0x4F);
    } else if(!strcmp(tokens[0], "JMP")) {
        if(ntokens != 2) error(nargsError);
        addByte(0x1F);
        addVal(tokens[1], T_WORD);
    } else if(!strcmp(tokens[0], "=")) {
        if(ntokens != 2) error(nargsError);
        if(!tryEval(tokens[1], &labels[nlabels-1].org)) error(evalError);
    } else if(!strcmp(tokens[0], "ALLOT")) {
        if(ntokens != 2) error(nargsError);
        if(!tryEval(tokens[1], &org)) error(evalError);
        org += labels[nlabels-1].org;
    } else {
        if(!strcmp(tokens[0], "-"))
            prev = org;
        else if(!strcmp(tokens[0], "+"))
            resAhead();
        else {
            if(tokens[0][0] != '.') {
                nlabels = lastNonLocal;
                lastNonLocal = nlabels+1;
            }
            strcpy(labels[nlabels].name, tokens[0]);
            labels[nlabels].org = org;
            nlabels++;
            resolve();
        }

        if(ntokens != 1) {
            for(i = 1; i < ntokens; i++) tokens[i-1] = tokens[i];
            ntokens--;
            goto assemble;
        }
    }

    /*while(oa < addr) printf("%.2X ", memory[oa++]);
    printf("\n");*/
}

int charNum(char *s, char c) {
    int n;
    n = 0;
    while(*s) if(*(s++) == c) n++;
    return n;
}

void asmFile(char *filename) {
    FILE *fp;
    unsigned int line;
    char buf[200];
    char *p;
    char c;

    fp = fopen(filename, "r");
    if(!fp) {
        if(g_filename)
            printf("error on line %d in %s:\n  failed to open %s\n",
                   g_line, g_filename, filename);
        else
            printf("error: failed to open %s\n", filename);
        exit(1);
    }

    g_filename = filename;
    g_line = 1;

    while(!feof(fp)) {
        p = buf;
        while((c = fgetc(fp)) != '\n' && !feof(fp))
            *(p++) = c;
        *p = 0;
        if((p = strchr(buf, ';')) && (p == buf || *(p-1) != '\'')) {
            *p = 0;
            if(charNum(buf, '"') % 2 != 0) *p = ';';
        }
        p = buf;
        while(*p && *p <= ' ') p++;
        if(p[0]) {
            line = g_line;
            asmLine(p);
            g_line = line;
            g_filename = filename;
        }
        g_line++;
    }

    fclose(fp);
}

void saveToFile(char *filename) {
    FILE *fp;
    fp = fopen(filename, "wb");
    if(!fp) {
        printf("failed to save to %s\n", filename);
        exit(1);
    }
    fwrite(memory, 1, addr, fp);
    fclose(fp);
}

void checkUnres() {
    int i;

    if(!nunres) return;

    printf("unresolved expressions:\n");

    for(i = 0; i < nunres; i++)
        printf("%s:%d\t%s\n",
               unres[i].filename, unres[i].line, unres[i].ex);

    error("unresolved expressions");
}

void printLabels() {
    for(int i = 0; i < nlabels; i++)
        printf("$%.8X %s\n", labels[i].org, labels[i].name);
}

int main(int argc, char **args) {
    if(argc != 3) {
        printf("usage: %s <file> <out>\n", args[0]);
        return 0;
    }

    asmFile(args[1]);
    checkUnres();
    printf("assembled successfully\n");
    saveToFile(args[2]);
    printf("wrote %s\n", args[2]);
    printLabels();
    return 0;
}
