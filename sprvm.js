// sprain vm js - tdwsl 2023

const MEMORY_SIZE = 16777216;
var memory = [];
//memory.fill(0, 0, MEMORY_SIZE);
for(var i = 0; i < MEMORY_SIZE; i++) memory[i] = 0;
var regs = [];
for(var i = 0; i < 16; i++) regs[i] = 0;
var debug = false;

function setb(addr, v) {
    v = Math.floor(v);
    v %= 0x100;
    memory[addr] = v;
}

function getb(addr) {
    var v = memory[addr];
    if((v&0x80) != 0) v = ~(v^0xff);
    return v;
}

function setm(addr, v) {
    v = Math.floor(v);
    if(v < 0) v = ~(v*-1)+1;
    v &= 0xffffffff;
    memory[addr] = v&0xff;
    memory[addr+1] = (v>>8)&0xff;
    memory[addr+2] = (v>>16)&0xff;
    memory[addr+3] = (v>>24)&0xff;
}

function getm(addr) {
    v = memory[addr] |
        (memory[addr+1]<<8) |
        (memory[addr+2]<<16) |
        (memory[addr+3]<<24);
    return v;
}

function push(v) {
    setm(regs[14], v);
    regs[14] += 4;
    regs[14] %= 0x100000000;
}

function pop(v) {
    regs[14] -= 4;
    regs[14] %= 0x100000000;
    return getm(regs[14]);
}

function hex(c) {
    var chars = [
        "0", "1", "2", "3", "4", "5", "6", "7",
        "8", "9", "A", "B", "C", "D", "E", "F",
    ];
    return chars[c];
}

function hexb(v) {
    v &= 0xff;
    return hex(v>>4).concat(hex(v&0x0f));
}

function hexw(v) {
    return hexb(v>>24)
      .concat(hexb(v>>16))
      .concat(hexb(v>>8))
      .concat(hexb(v));
}

function loadProgram(arr) {
    regs[15] = arr[0] | (arr[1]<<8) | (arr[2]<<16) | (arr[3]<<24);
    regs[14] = 0;
    for(var i = 4; i < arr.length; i++)
        memory[i-4+regs[15]] = arr[i];
}

function printIns(pc) {
    var insStrs = ["", "MOV", "ADD", "PUSH", "POP", "INV", "SHR", "SHL"];
    var linsStrs = [
        "INT", "CALL", "BEQ", "BNE", "BGE", "BLT", "MOV", "MOV",
        "MOV", "MOV", "MOV", "AND", "OR", "XOR", "ADD", "SUB",
    ];
    var regStrs = [
        "ZERO", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
        "R8", "R9", "R10", "R11", "R12", "R13", "RSP", "RPC",
    ];
    var lregStrs = [
        "LZERO", "L1", "L2", "L3", "L4", "L5", "L6", "L7",
        "L8", "L9", "L10", "L11", "L12", "L13", "LSP", "LPC",
    ];
    var nregStrs = [
        "(ZERO)", "(R1)", "(R2)", "(R3)", "(R4)", "(R5)", "(R6)", "(R7)",
        "(R8)", "(R9)", "(R10)", "(R11)", "(R12)", "(R13)", "(RSP)", "(RPC)",
    ];
    var ins = hexw(pc).concat(" ");
    if((memory[pc]&0xf0) == 0) ins = ins.concat(linsStrs[memory[pc]]);
    else ins = ins.concat(insStrs[memory[pc]>>4]);
    ins = ins.concat(" ");
    switch(memory[pc]&0xf0) {
    case 0x00:
        switch(memory[pc]) {
        case 0x00:
            ins = ins.concat(hexb(memory[pc+1]));
            break;
        case 0x01:
            ins = ins.concat(hexw(getm(pc+1)));
            break;
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
            ins = ins.concat(regStrs[memory[pc+1]>>4])
              .concat(",")
              .concat(regStrs[memory[pc+1]&0x0f])
              .concat(" ")
              .concat(hexb(memory[pc+2]));
            break;
        case 0x06:
            ins = ins.concat(regStrs[memory[pc+1]>>4])
              .concat(",")
              .concat(nregStrs[memory[pc+1]&0x0f]);
            break;
        case 0x07:
            ins = ins.concat(nregStrs[memory[pc+1]>>4])
              .concat(",")
              .concat(regStrs[memory[pc+1]&0x0f]);
            break;
        case 0x08:
            ins = ins.concat(lregStrs[memory[pc+1]>>4])
              .concat(",")
              .concat(nregStrs[memory[pc+1]&0x0f]);
            break;
        case 0x09:
            ins = ins.concat(nregStrs[memory[pc+1]>>4])
              .concat(",")
              .concat(lregStrs[memory[pc+1]&0x0f]);
            break;
        case 0x0a:
        case 0x0b:
        case 0x0c:
        case 0x0d:
        case 0x0e:
        case 0x0f:
            ins = ins.concat(regStrs[memory[pc+1]>>4])
              .concat(",")
              .concat(regStrs[memory[pc+1]&0x0f]);
            break;
        }
        break;
    case 0x10:
        ins = ins.concat(regStrs[memory[pc]&0x0f])
          .concat(",")
          .concat(hexw(getm(pc+1)));
        break;
    case 0x20:
    case 0x60:
    case 0x70:
        ins = ins.concat(regStrs[memory[pc]&0x0f])
          .concat(",")
          .concat(hexb(memory[pc+1]));
        break;
    case 0x30:
    case 0x40:
    case 0x50:
        ins = ins.concat(regStrs[memory[pc]&0x0f]);
        break;
    }
    console.log(ins);
}

function printRegs() {
    var str = "";
    for(var i = 1; i < 15; i++)
        str = str.concat(hexw(regs[i])).concat(" ");
    console.log(str);
}

function run() {
    var ins;
    for(;;) {
        if(debug) {
            printRegs();
            printIns(regs[15]);
        }
        ins = memory[regs[15]++];
        switch(ins&0xf0) {
        case 0x00:
            switch(ins) {
            case 0x00:
                return memory[regs[15]++];
            case 0x01:
                regs[15] += 4;
                push(regs[15]);
                regs[15] = getm(regs[15]-4);
                break;
            case 0x02:
                ins = memory[regs[15]];
                regs[15] += 2;
                if(regs[ins>>4] == regs[ins&0x0f])
                    regs[15] += getb(regs[15]-1);
                break;
            case 0x03:
                ins = memory[regs[15]];
                regs[15] += 2;
                if(regs[ins>>4] != regs[ins&0x0f])
                    regs[15] += getb(regs[15]-1);
                break;
            case 0x04:
                ins = memory[regs[15]];
                regs[15] += 2;
                if((((regs[ins>>4]-regs[ins&0x0f])
                  %0x100000000)&0x80000000) == 0)
                    regs[15] += getb(regs[15]-1);
                break;
            case 0x05:
                ins = memory[regs[15]];
                regs[15] += 2;
                if((((regs[ins>>4]-regs[ins&0x0f])
                  %0x100000000)&0x80000000) != 0)
                    regs[15] += getb(regs[15]-1);
                break;
            case 0x06:
                ins = memory[regs[15]++];
                regs[ins>>4] = getm(regs[ins&0x0f]);
                break;
            case 0x07:
                ins = memory[regs[15]++];
                setm(regs[ins>>4], regs[ins&0x0f]);
                break;
            case 0x08:
                ins = memory[regs[15]++];
                regs[ins>>4] = memory[regs[ins&0x0f]];
                break;
            case 0x09:
                ins = memory[regs[15]++];
                memory[regs[ins>>4]] = regs[ins&0x0f]&0xff;
                break;
            case 0x0a:
                ins = memory[regs[15]++];
                regs[ins>>4] = regs[ins&0x0f];
                break;
            case 0x0b:
                ins = memory[regs[15]++];
                regs[ins>>4] &= regs[ins&0x0f];
                break;
            case 0x0c:
                ins = memory[regs[15]++];
                regs[ins>>4] |= regs[ins&0x0f];
                break;
            case 0x0d:
                ins = memory[regs[15]++];
                regs[ins>>4] ^= regs[ins&0x0f];
                break;
            case 0x0e:
                ins = memory[regs[15]++];
                regs[ins>>4] += regs[ins&0x0f];
                regs[ins>>4] %= 0x100000000;
                break;
            case 0x0f:
                ins = memory[regs[15]++];
                regs[ins>>4] -= regs[ins&0x0f];
                regs[ins>>4] %= 0x100000000;
                break;
            }
            break;
        case 0x10:
            regs[15] += 4;
            regs[ins&0x0f] = getm(regs[15]-4);
            break;
        case 0x20:
            regs[15]++;
            regs[ins&0x0f] += getb(regs[15]-1);
            regs[ins&0x0f] %= 0x100000000;
            break;
        case 0x30:
            push(regs[ins&0x0f]);
            break;
        case 0x40:
            regs[ins&0x0f] = pop();
            break;
        case 0x50:
            regs[ins&0x0f] = ~regs[ins>>4];
            break;
        case 0x60:
            regs[15]++;
            regs[ins&0x0f] >>= memory[regs[15]-1];
            break;
        case 0x70:
            regs[15]++;
            regs[ins&0x0f] <<= memory[regs[15]-1];
            break;
        }
    }
    return -1;
}
