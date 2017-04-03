8150 // PC keyboard interface constants
8151 
8152 #define KBSTATP         0x64    // kbd controller status port(I)
8153 #define KBS_DIB         0x01    // kbd data in buffer
8154 #define KBDATAP         0x60    // kbd data port(I)
8155 
8156 #define NO              0
8157 
8158 #define SHIFT           (1<<0)
8159 #define CTL             (1<<1)
8160 #define ALT             (1<<2)
8161 
8162 #define CAPSLOCK        (1<<3)
8163 #define NUMLOCK         (1<<4)
8164 #define SCROLLLOCK      (1<<5)
8165 
8166 #define E0ESC           (1<<6)
8167 
8168 // Special keycodes
8169 #define KEY_HOME        0xE0
8170 #define KEY_END         0xE1
8171 #define KEY_UP          0xE2
8172 #define KEY_DN          0xE3
8173 #define KEY_LF          0xE4
8174 #define KEY_RT          0xE5
8175 #define KEY_PGUP        0xE6
8176 #define KEY_PGDN        0xE7
8177 #define KEY_INS         0xE8
8178 #define KEY_DEL         0xE9
8179 
8180 // C('A') == Control-A
8181 #define C(x) (x - '@')
8182 
8183 static uchar shiftcode[256] =
8184 {
8185   [0x1D] CTL,
8186   [0x2A] SHIFT,
8187   [0x36] SHIFT,
8188   [0x38] ALT,
8189   [0x9D] CTL,
8190   [0xB8] ALT
8191 };
8192 
8193 static uchar togglecode[256] =
8194 {
8195   [0x3A] CAPSLOCK,
8196   [0x45] NUMLOCK,
8197   [0x46] SCROLLLOCK
8198 };
8199 
8200 static uchar normalmap[256] =
8201 {
8202   NO,   0x1B, '1',  '2',  '3',  '4',  '5',  '6',  // 0x00
8203   '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
8204   'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10
8205   'o',  'p',  '[',  ']',  '\n', NO,   'a',  's',
8206   'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 0x20
8207   '\'', '`',  NO,   '\\', 'z',  'x',  'c',  'v',
8208   'b',  'n',  'm',  ',',  '.',  '/',  NO,   '*',  // 0x30
8209   NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
8210   NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
8211   '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
8212   '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
8213   [0x9C] '\n',      // KP_Enter
8214   [0xB5] '/',       // KP_Div
8215   [0xC8] KEY_UP,    [0xD0] KEY_DN,
8216   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
8217   [0xCB] KEY_LF,    [0xCD] KEY_RT,
8218   [0x97] KEY_HOME,  [0xCF] KEY_END,
8219   [0xD2] KEY_INS,   [0xD3] KEY_DEL
8220 };
8221 
8222 static uchar shiftmap[256] =
8223 {
8224   NO,   033,  '!',  '@',  '#',  '$',  '%',  '^',  // 0x00
8225   '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
8226   'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  // 0x10
8227   'O',  'P',  '{',  '}',  '\n', NO,   'A',  'S',
8228   'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  // 0x20
8229   '"',  '~',  NO,   '|',  'Z',  'X',  'C',  'V',
8230   'B',  'N',  'M',  '<',  '>',  '?',  NO,   '*',  // 0x30
8231   NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
8232   NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
8233   '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
8234   '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
8235   [0x9C] '\n',      // KP_Enter
8236   [0xB5] '/',       // KP_Div
8237   [0xC8] KEY_UP,    [0xD0] KEY_DN,
8238   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
8239   [0xCB] KEY_LF,    [0xCD] KEY_RT,
8240   [0x97] KEY_HOME,  [0xCF] KEY_END,
8241   [0xD2] KEY_INS,   [0xD3] KEY_DEL
8242 };
8243 
8244 
8245 
8246 
8247 
8248 
8249 
8250 static uchar ctlmap[256] =
8251 {
8252   NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
8253   NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
8254   C('Q'),  C('W'),  C('E'),  C('R'),  C('T'),  C('Y'),  C('U'),  C('I'),
8255   C('O'),  C('P'),  NO,      NO,      '\r',    NO,      C('A'),  C('S'),
8256   C('D'),  C('F'),  C('G'),  C('H'),  C('J'),  C('K'),  C('L'),  NO,
8257   NO,      NO,      NO,      C('\\'), C('Z'),  C('X'),  C('C'),  C('V'),
8258   C('B'),  C('N'),  C('M'),  NO,      NO,      C('/'),  NO,      NO,
8259   [0x9C] '\r',      // KP_Enter
8260   [0xB5] C('/'),    // KP_Div
8261   [0xC8] KEY_UP,    [0xD0] KEY_DN,
8262   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
8263   [0xCB] KEY_LF,    [0xCD] KEY_RT,
8264   [0x97] KEY_HOME,  [0xCF] KEY_END,
8265   [0xD2] KEY_INS,   [0xD3] KEY_DEL
8266 };
8267 
8268 
8269 
8270 
8271 
8272 
8273 
8274 
8275 
8276 
8277 
8278 
8279 
8280 
8281 
8282 
8283 
8284 
8285 
8286 
8287 
8288 
8289 
8290 
8291 
8292 
8293 
8294 
8295 
8296 
8297 
8298 
8299 
