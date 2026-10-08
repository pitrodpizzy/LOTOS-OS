/*
 * LOTOS OS
 * USB 2.0 EHCI Host Controller
 *
 * Etap:
 *  - PCI scan
 *  - EHCI detection
 *  - EHCI initialization
 *  - asynchronous schedule
 *  - Queue Head (QH)
 *  - Queue Transfer Descriptor (qTD)
 *  - Bulk IN
 *  - Bulk OUT
 *
 * Założenia:
 *  - x86 32-bit
 *  - freestanding kernel
 *  - identity mapping pamięci fizycznej
 *  - brak DMA/IOMMU
 *  - adresy struktur EHCI są fizycznymi adresami
 */

#include <stdint.h>

/* ============================================================
 * VGA
 * ============================================================ */

#define VGA_MEMORY ((volatile uint16_t*)0xB8000)

static int usb_cursor = 0;

static void usb_putc(char c)
{
    if (c == '\n') {
        usb_cursor += 80 - (usb_cursor % 80);
        return;
    }

    VGA_MEMORY[usb_cursor++] =
        ((uint16_t)0x07 << 8) | (uint8_t)c;

    if (usb_cursor >= 80 * 25)
        usb_cursor = 0;
}

static void usb_print(const char *s)
{
    while (*s)
        usb_putc(*s++);
}

static void usb_print_hex(uint32_t value)
{
    const char *hex = "0123456789ABCDEF";

    usb_putc('0');
    usb_putc('x');

    for (int i = 7; i >= 0; i--)
        usb_putc(hex[(value >> (i * 4)) & 0xF]);
}

static void usb_print_dec(uint32_t value)
{
    char buf[12];
    int i = 0;

    if (value == 0) {
        usb_putc('0');
        return;
    }

    while (value > 0 && i < 11) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i--)
        usb_putc(buf[i]);
}


/* ============================================================
 * IO PORTS
 * ============================================================ */

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;

    __asm__ volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t value;

    __asm__ volatile (
        "inl %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}

static inline void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile (
        "outl %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}


/* ============================================================
 * PCI
 * ============================================================ */

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static uint32_t pci_read32(
    uint8_t bus,
    uint8_t slot,
    uint8_t function,
    uint8_t offset
)
{
    uint32_t address;

    address =
        (1u << 31) |
        ((uint32_t)bus << 16) |
        ((uint32_t)slot << 11) |
        ((uint32_t)function << 8) |
        (offset & 0xFC);

    outl(PCI_CONFIG_ADDRESS, address);

    return inl(PCI_CONFIG_DATA);
}

static void pci_write32(
    uint8_t bus,
    uint8_t slot,
    uint8_t function,
    uint8_t offset,
    uint32_t value
)
{
    uint32_t address;

    address =
        (1u << 31) |
        ((uint32_t)bus << 16) |
        ((uint32_t)slot << 11) |
        ((uint32_t)function << 8) |
        (offset & 0xFC);

    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}


/* ============================================================
 * EHCI CONSTANTS
 * ============================================================ */

/*
 * PCI:
 *
 * Class       = 0x0C
 * Subclass    = 0x03
 * ProgIF      = 0x20 -> EHCI
 */

#define PCI_CLASS_SERIAL_BUS       0x0C
#define PCI_SUBCLASS_USB           0x03
#define PCI_PROGIF_EHCI            0x20

#define PCI_COMMAND_OFFSET         0x04
#define PCI_BAR0_OFFSET            0x10

#define PCI_COMMAND_IO             (1 << 0)
#define PCI_COMMAND_MEMORY         (1 << 1)
#define PCI_COMMAND_BUSMASTER      (1 << 2)


/* EHCI capability registers */

#define EHCI_CAPLENGTH             0x00
#define EHCI_HCSPARAMS             0x04
#define EHCI_HCCPARAMS             0x08


/* EHCI operational registers */

#define EHCI_USBCMD                0x00
#define EHCI_USBSTS                0x04
#define EHCI_USBINTR               0x08
#define EHCI_FRINDEX               0x0C
#define EHCI_CTRLDSSEGMENT         0x10
#define EHCI_PERIODICLISTBASE      0x14
#define EHCI_ASYNCLISTADDR         0x18
#define EHCI_CONFIGFLAG            0x40
#define EHCI_PORTSC                0x44


/* USBCMD */

#define EHCI_CMD_RUN               (1 << 0)
#define EHCI_CMD_RESET             (1 << 1)
#define EHCI_CMD_ASE               (1 << 5)


/* USBSTS */

#define EHCI_STS_HALTED            (1 << 12)
#define EHCI_STS_ASE               (1 << 15)


/* PORTSC */

#define EHCI_PORT_CCS              (1 << 0)
#define EHCI_PORT_CSC              (1 << 1)
#define EHCI_PORT_PE               (1 << 2)
#define EHCI_PORT_PEC              (1 << 3)
#define EHCI_PORT_OCA              (1 << 4)
#define EHCI_PORT_OCC              (1 << 5)
#define EHCI_PORT_RESET            (1 << 8)
#define EHCI_PORT_OWNER            (1 << 13)


/* ============================================================
 * EHCI qTD
 * ============================================================ */

/*
 * qTD:
 *
 * next
 * alt_next
 * token
 * buffer[5]
 * buffer_hi[5]
 *
 * EHCI wymaga odpowiedniego wyrównania struktur.
 */

#define EHCI_PTR_TERMINATE         0x00000001

#define QTD_PID_OUT                (0 << 8)
#define QTD_PID_IN                 (1 << 8)
#define QTD_PID_SETUP              (2 << 8)

#define QTD_STATUS_ACTIVE          (1 << 7)
#define QTD_STATUS_HALTED          (1 << 6)
#define QTD_STATUS_DATABUF         (1 << 5)
#define QTD_STATUS_BABBLE          (1 << 4)
#define QTD_STATUS_XACTERR         (1 << 3)

#define QTD_CERR_3                 (3 << 10)

#define QTD_IOC                    (1 << 15)

#define QTD_TOGGLE                 (1u << 31)

#define QTD_LENGTH(x)              (((x) & 0x7FFF0000) >> 16)


struct ehci_qtd {
    uint32_t next;
    uint32_t alt_next;
    uint32_t token;

    uint32_t buffer[5];

    uint32_t buffer_hi[5];

} __attribute__((aligned(32)));


/* ============================================================
 * EHCI Queue Head
 * ============================================================ */

struct ehci_qh {

    uint32_t horizontal_link;

    uint32_t endpoint_char;

    uint32_t endpoint_caps;

    uint32_t current_qtd;

    /*
     * Overlay
     */

    uint32_t next_qtd;
    uint32_t alt_next_qtd;
    uint32_t token;

    uint32_t buffer[5];

    uint32_t buffer_hi[5];

} __attribute__((aligned(32)));


/* ============================================================
 * EHCI GLOBAL DATA
 * ============================================================ */

static volatile uint8_t *ehci_mmio = 0;

static uint32_t ehci_operational = 0;

static uint8_t ehci_bus = 0;
static uint8_t ehci_slot = 0;
static uint8_t ehci_function = 0;

static uint8_t ehci_ports = 0;


/*
 * Jedna QH.
 *
 * Na początek wystarczy jeden endpoint.
 */

static struct ehci_qh
    ehci_bulk_qh __attribute__((aligned(32)));


/*
 * qTD-y.
 *
 * 8 sztuk pozwala obsłużyć większe transfery.
 */

#define EHCI_MAX_QTDS 8

static struct ehci_qtd
    ehci_qtds[EHCI_MAX_QTDS]
    __attribute__((aligned(32)));


/*
 * Bufor pomocniczy.
 *
 * Przydaje się do diagnostyki.
 */

static uint8_t
    ehci_scratch[4096]
    __attribute__((aligned(4096)));


/* ============================================================
 * MMIO
 * ============================================================ */

static inline uint32_t ehci_read32(uint32_t offset)
{
    volatile uint32_t *reg =
        (volatile uint32_t *)(ehci_mmio + offset);

    return *reg;
}

static inline void ehci_write32(
    uint32_t offset,
    uint32_t value
)
{
    volatile uint32_t *reg =
        (volatile uint32_t *)(ehci_mmio + offset);

    *reg = value;
}


/* ============================================================
 * DELAY
 * ============================================================ */

/*
 * Bardzo prosty delay.
 *
 * Później zastąpimy go PIT-em.
 */

static void ehci_delay(void)
{
    for (volatile uint32_t i = 0;
         i < 100000;
         i++)
    {
        __asm__ volatile ("pause");
    }
}

static void ehci_delay_long(void)
{
    for (int i = 0; i < 20; i++)
        ehci_delay();
}


/* ============================================================
 * RESET CONTROLLER
 * ============================================================ */

static int ehci_controller_reset(void)
{
    uint32_t cmd;

    usb_print("EHCI: controller reset...\n");

    cmd = ehci_read32(EHCI_USBCMD);

    /*
     * zatrzymaj kontroler
     */
    cmd &= ~EHCI_CMD_RUN;

    ehci_write32(EHCI_USBCMD, cmd);

    ehci_delay();

    /*
     * reset
     */
    cmd = ehci_read32(EHCI_USBCMD);

    cmd |= EHCI_CMD_RESET;

    ehci_write32(EHCI_USBCMD, cmd);

    /*
     * czekamy aż sprzęt wyczyści RESET
     */
    for (uint32_t i = 0; i < 1000000; i++) {

        cmd = ehci_read32(EHCI_USBCMD);

        if (!(cmd & EHCI_CMD_RESET))
            break;

        __asm__ volatile ("pause");
    }

    cmd = ehci_read32(EHCI_USBCMD);

    if (cmd & EHCI_CMD_RESET) {

        usb_print("EHCI: reset timeout!\n");

        return -1;
    }

    usb_print("EHCI: reset OK\n");

    return 0;
}


/* ============================================================
 * PORT RESET
 * ============================================================ */

static int ehci_reset_port(uint8_t port)
{
    uint32_t offset;
    uint32_t status;

    offset =
        EHCI_PORTSC +
        ((uint32_t)port * 4);

    status = ehci_read32(offset);

    /*
     * Jeżeli port nie ma urządzenia
     */
    if (!(status & EHCI_PORT_CCS)) {

        usb_print("  Port ");
        usb_print_dec(port + 1);
        usb_print(": empty\n");

        return 0;
    }

    usb_print("  Port ");
    usb_print_dec(port + 1);
    usb_print(": device detected\n");


    /*
     * Jeżeli port jest przekazany do
     * UHCI/OHCI, EHCI go nie obsługuje.
     */

    if (status & EHCI_PORT_OWNER) {

        usb_print("    owned by companion controller\n");

        return -1;
    }


    /*
     * Najpierw wyczyść R/WC.
     */

    status &= ~(
        EHCI_PORT_CSC |
        EHCI_PORT_PEC |
        EHCI_PORT_OCC
    );

    /*
     * Ustaw reset.
     */

    status |= EHCI_PORT_RESET;

    ehci_write32(offset, status);

    /*
     * USB reset powinien potrwać minimum
     * odpowiedni czas.
     *
     * Nasz delay jest obecnie przybliżony.
     */

    ehci_delay_long();


    /*
     * EHCI wymaga wyłączenia bitu reset
     * przez software.
     */

    status = ehci_read32(offset);

    status &= ~EHCI_PORT_RESET;

    ehci_write32(offset, status);


    /*
     * chwilę czekamy
     */

    ehci_delay();


    status = ehci_read32(offset);

    usb_print("    PORTSC = ");
    usb_print_hex(status);
    usb_putc('\n');


    if (status & EHCI_PORT_PE) {

        usb_print("    High-Speed port enabled\n");

        return 1;
    }

    usb_print("    port not enabled\n");

    return 0;
}


/* ============================================================
 * QTD BUFFER SETUP
 * ============================================================ */

/*
 * EHCI qTD ma maksymalnie 5 wpisów stron.
 *
 * Każdy wpis wskazuje stronę 4 KiB.
 */

static void ehci_qtd_set_buffer(
    struct ehci_qtd *qtd,
    uint32_t address,
    uint32_t length
)
{
    uint32_t page;
    uint32_t i;

    page = address & ~0xFFF;

    qtd->buffer[0] = address;

    for (i = 1; i < 5; i++) {

        page += 0x1000;

        qtd->buffer[i] = page;
    }

    /*
     * LOTOS jest obecnie 32-bitowy,
     * więc high DWORD = 0.
     */

    for (i = 0; i < 5; i++)
        qtd->buffer_hi[i] = 0;

    (void)length;
}


/* ============================================================
 * QTD INIT
 * ============================================================ */

static void ehci_qtd_init(
    struct ehci_qtd *qtd,
    uint32_t buffer,
    uint32_t length,
    int direction,
    int toggle,
    int ioc
)
{
    uint32_t token;

    qtd->next = EHCI_PTR_TERMINATE;

    qtd->alt_next = EHCI_PTR_TERMINATE;

    ehci_qtd_set_buffer(
        qtd,
        buffer,
        length
    );

    /*
     * 3 retry attempts
     */

    token = QTD_CERR_3;

    /*
     * długość transferu
     */

    token |= (length & 0x7FFF) << 16;


    /*
     * PID
     */

    if (direction)
        token |= QTD_PID_IN;
    else
        token |= QTD_PID_OUT;


    /*
     * toggle
     */

    if (toggle)
        token |= QTD_TOGGLE;


    /*
     * interrupt on complete
     */

    if (ioc)
        token |= QTD_IOC;


    /*
     * ACTIVE musi być ustawione
     * zanim kontroler zobaczy qTD.
     */

    token |= QTD_STATUS_ACTIVE;

    qtd->token = token;
}


/* ============================================================
 * QH INIT
 * ============================================================ */

/*
 * endpoint:
 *     endpoint USB, np. 1
 *
 * device:
 *     USB device address
 *
 * max_packet:
 *     np. 512 dla High-Speed Bulk
 */

static void ehci_qh_init(
    struct ehci_qh *qh,
    uint8_t device,
    uint8_t endpoint,
    uint16_t max_packet
)
{
    uint32_t ep_char;

    qh->horizontal_link =
        ((uint32_t)qh) | 0x2;

    /*
     * Device Address
     *
     * bits 0-6
     */

    ep_char = device & 0x7F;

    /*
     * Endpoint Number
     *
     * bits 8-11
     */

    ep_char |=
        ((uint32_t)(endpoint & 0x0F)) << 8;

    /*
     * EPS = High Speed
     *
     * bits 12-13
     *
     * 2 = High Speed
     */

    ep_char |=
        (2u << 12);


    /*
     * Maximum Packet Length
     *
     * bits 16-26
     */

    ep_char |=
        ((uint32_t)(max_packet & 0x7FF)) << 16;


    /*
     * QH Head of Reclamation List
     * nie ustawiamy.
     */

    qh->endpoint_char = ep_char;


    /*
     * Endpoint Capabilities
     *
     * MULT = 1
     */

    qh->endpoint_caps = 1u << 30;


    /*
     * Current qTD
     */

    qh->current_qtd =
        EHCI_PTR_TERMINATE;


    /*
     * overlay
     */

    qh->next_qtd =
        EHCI_PTR_TERMINATE;

    qh->alt_next_qtd =
        EHCI_PTR_TERMINATE;

    qh->token = 0;


    for (int i = 0; i < 5; i++) {

        qh->buffer[i] = 0;
        qh->buffer_hi[i] = 0;
    }
}


/* ============================================================
 * ASYNC SCHEDULE
 * ============================================================ */

static int ehci_start_async(
    struct ehci_qh *qh
)
{
    uint32_t cmd;

    /*
     * EHCI wymaga adresu QH wyrównanego
     * do 32 bajtów.
     */

    if (((uint32_t)qh & 0x1F) != 0) {

        usb_print(
            "EHCI: QH alignment error!\n"
        );

        return -1;
    }


    /*
     * Async list address.
     */

    ehci_write32(
        EHCI_ASYNCLISTADDR,
        ((uint32_t)qh) & ~0x1F
    );


    /*
     * CONFIGFLAG = 1
     *
     * routing portów do EHCI.
     */

    ehci_write32(
        EHCI_CONFIGFLAG,
        1
    );


    /*
     * Włącz asynchronous schedule.
     */

    cmd = ehci_read32(EHCI_USBCMD);

    cmd |= EHCI_CMD_ASE;

    /*
     * Uruchom kontroler.
     */

    cmd |= EHCI_CMD_RUN;

    ehci_write32(
        EHCI_USBCMD,
        cmd
    );


    /*
     * poczekaj
     */

    for (uint32_t i = 0;
         i < 1000000;
         i++)
    {
        uint32_t status;

        status =
            ehci_read32(EHCI_USBSTS);

        if (status & EHCI_STS_ASE)
            break;

        __asm__ volatile ("pause");
    }


    usb_print(
        "EHCI: async schedule started\n"
    );

    return 0;
}


/* ============================================================
 * STOP ASYNC SCHEDULE
 * ============================================================ */

static void ehci_stop_async(void)
{
    uint32_t cmd;

    cmd = ehci_read32(EHCI_USBCMD);

    cmd &= ~EHCI_CMD_ASE;

    ehci_write32(
        EHCI_USBCMD,
        cmd
    );

    ehci_delay();

    cmd = ehci_read32(EHCI_USBCMD);

    cmd &= ~EHCI_CMD_RUN;

    ehci_write32(
        EHCI_USBCMD,
        cmd
    );
}


/* ============================================================
 * WAIT FOR qTD
 * ============================================================ */

static int ehci_wait_qtd(
    struct ehci_qtd *qtd
)
{
    for (uint32_t timeout = 0;
         timeout < 5000000;
         timeout++)
    {
        uint32_t token;

        token = qtd->token;


        /*
         * Nadal aktywny
         */

        if (token & QTD_STATUS_ACTIVE) {

            __asm__ volatile ("pause");

            continue;
        }


        /*
         * HALTED
         */

        if (token & QTD_STATUS_HALTED) {

            usb_print(
                "EHCI: qTD HALTED token="
            );

            usb_print_hex(token);

            usb_putc('\n');

            return -1;
        }


        /*
         * Data buffer error
         */

        if (token & QTD_STATUS_DATABUF) {

            usb_print(
                "EHCI: qTD data buffer error\n"
            );

            return -2;
        }


        /*
         * Babble
         */

        if (token & QTD_STATUS_BABBLE) {

            usb_print(
                "EHCI: qTD babble error\n"
            );

            return -3;
        }


        /*
         * Transaction error
         */

        if (token & QTD_STATUS_XACTERR) {

            usb_print(
                "EHCI: qTD transaction error\n"
            );

            return -4;
        }


        return 0;
    }


    usb_print(
        "EHCI: qTD timeout\n"
    );

    return -5;
}


/* ============================================================
 * BULK TRANSFER
 * ============================================================ */

/*
 * direction:
 *
 * 0 = OUT
 * 1 = IN
 *
 * device:
 *     USB device address
 *
 * endpoint:
 *     Bulk endpoint number
 *
 * max_packet:
 *     np. 512
 *
 * buffer:
 *     adres fizyczny RAM
 *
 * length:
 *     liczba bajtów
 *
 * toggle:
 *     aktualny DATA0/DATA1
 */

int ehci_bulk_transfer(
    uint8_t device,
    uint8_t endpoint,
    uint16_t max_packet,
    int direction,
    void *buffer,
    uint32_t length,
    int *toggle
)
{
    uint32_t address;
    uint32_t remaining;

    int current_toggle;

    if (!buffer || length == 0)
        return -1;


    /*
     * LOTOS na tym etapie zakłada:
     *
     * virtual address == physical address
     */

    address = (uint32_t)buffer;

    remaining = length;

    current_toggle =
        toggle ? *toggle : 0;


    /*
     * Jedna operacja może użyć kilku qTD.
     *
     * Każdy qTD może wskazać maksymalnie
     * około 20 KiB danych przez 5 stron.
     */

    uint32_t qcount = 0;

    uint32_t temp_address = address;

    uint32_t temp_remaining = remaining;


    while (temp_remaining > 0) {

        if (qcount >= EHCI_MAX_QTDS) {

            usb_print(
                "EHCI: too many qTDs\n"
            );

            return -2;
        }


        uint32_t page_offset =
            temp_address & 0xFFF;

        uint32_t capacity =
            0x5000 - page_offset;


        if (capacity > temp_remaining)
            capacity = temp_remaining;


        /*
         * qTD length ma 15 bitów,
         * ale tutaj ograniczamy go
         * dodatkowo do 5 stron.
         */

        if (capacity > 0x5000)
            capacity = 0x5000;


        /*
         * Liczba pakietów.
         *
         * Potrzebujemy jej do ustalenia
         * końcowego DATA TOGGLE.
         */

        uint32_t packets =
            (capacity + max_packet - 1)
            / max_packet;


        /*
         * Ostatni qTD dostaje IOC.
         */

        int ioc =
            (temp_remaining <= capacity);


        ehci_qtd_init(
            &ehci_qtds[qcount],
            temp_address,
            capacity,
            direction,
            current_toggle,
            ioc
        );


        /*
         * następny qTD
         */

        if (!ioc) {

            ehci_qtds[qcount].next =
                ((uint32_t)
                    &ehci_qtds[qcount + 1])
                & ~0x1F;
        }


        /*
         * DATA0/DATA1 zmienia się
         * po każdym pakiecie.
         */

        if (packets & 1)
            current_toggle ^= 1;


        temp_address += capacity;
        temp_remaining -= capacity;

        qcount++;
    }


    /*
     * Ostatni qTD.
     */

    ehci_qtds[qcount - 1].next =
        EHCI_PTR_TERMINATE;


    /*
     * Wyczyść QH.
     */

    ehci_qh_init(
        &ehci_bulk_qh,
        device,
        endpoint,
        max_packet
    );


    /*
     * QH wskazuje na pierwszy qTD.
     */

    ehci_bulk_qh.next_qtd =
        ((uint32_t)&ehci_qtds[0])
        & ~0x1F;


    ehci_bulk_qh.alt_next_qtd =
        EHCI_PTR_TERMINATE;


    /*
     * Włącz transfer.
     */

    ehci_bulk_qh.token = 0;


    /*
     * Uruchom async schedule.
     */

    if (ehci_start_async(
            &ehci_bulk_qh) != 0)
    {
        return -3;
    }


    /*
     * Czekamy na ostatni qTD.
     */

    int result =
        ehci_wait_qtd(
            &ehci_qtds[qcount - 1]
        );


    /*
     * Zatrzymujemy async schedule.
     *
     * Na tym etapie robimy to po każdym
     * transferze dla prostoty.
     */

    ehci_stop_async();


    /*
     * Aktualizuj toggle.
     */

    if (result == 0 && toggle)
        *toggle = current_toggle;


    return result;
}


/* ============================================================
 * BULK OUT
 * ============================================================ */

int ehci_bulk_out(
    uint8_t device,
    uint8_t endpoint,
    uint16_t max_packet,
    void *buffer,
    uint32_t length,
    int *toggle
)
{
    return ehci_bulk_transfer(
        device,
        endpoint,
        max_packet,
        0,
        buffer,
        length,
        toggle
    );
}


/* ============================================================
 * BULK IN
 * ============================================================ */

int ehci_bulk_in(
    uint8_t device,
    uint8_t endpoint,
    uint16_t max_packet,
    void *buffer,
    uint32_t length,
    int *toggle
)
{
    return ehci_bulk_transfer(
        device,
        endpoint,
        max_packet,
        1,
        buffer,
        length,
        toggle
    );
}


/* ============================================================
 * EHCI PCI DETECTION
 * ============================================================ */

static int ehci_find_controller(void)
{
    for (uint16_t bus = 0;
         bus < 256;
         bus++)
    {
        for (uint8_t slot = 0;
             slot < 32;
             slot++)
        {
            for (uint8_t function = 0;
                 function < 8;
                 function++)
            {
                uint32_t id;
                uint32_t class_reg;

                id = pci_read32(
                    bus,
                    slot,
                    function,
                    0x00
                );


                /*
                 * 0xFFFFFFFF = brak urządzenia
                 */

                if (id == 0xFFFFFFFF)
                    continue;


                class_reg =
                    pci_read32(
                        bus,
                        slot,
                        function,
                        0x08
                    );


                uint8_t class_code =
                    (class_reg >> 24) & 0xFF;

                uint8_t subclass =
                    (class_reg >> 16) & 0xFF;

                uint8_t prog_if =
                    (class_reg >> 8) & 0xFF;


                if (
                    class_code ==
                        PCI_CLASS_SERIAL_BUS &&
                    subclass ==
                        PCI_SUBCLASS_USB &&
                    prog_if ==
                        PCI_PROGIF_EHCI
                )
                {
                    ehci_bus =
                        (uint8_t)bus;

                    ehci_slot =
                        slot;

                    ehci_function =
                        function;


                    usb_print(
                        "EHCI controller found: "
                    );

                    usb_print_dec(bus);

                    usb_putc(':');

                    usb_print_dec(slot);

                    usb_putc('.');

                    usb_print_dec(function);

                    usb_putc('\n');


                    return 0;
                }
            }
        }
    }


    usb_print(
        "EHCI controller not found\n"
    );

    return -1;
}


/* ============================================================
 * EHCI INITIALIZATION
 * ============================================================ */

int usb_ehci_init(void)
{
    uint32_t bar0;
    uint32_t command;

    usb_print(
        "\n=== LOTOS USB EHCI ===\n"
    );


    /*
     * Znajdź EHCI na PCI.
     */

    if (ehci_find_controller() != 0)
        return -1;


    /*
     * PCI Command.
     *
     * Włącz:
     * Memory Space
     * Bus Master
     */

    command =
        pci_read32(
            ehci_bus,
            ehci_slot,
            ehci_function,
            PCI_COMMAND_OFFSET
        );


    command |=
        PCI_COMMAND_MEMORY |
        PCI_COMMAND_BUSMASTER;


    pci_write32(
        ehci_bus,
        ehci_slot,
        ehci_function,
        PCI_COMMAND_OFFSET,
        command
    );


    /*
     * BAR0
     */

    bar0 =
        pci_read32(
            ehci_bus,
            ehci_slot,
            ehci_function,
            PCI_BAR0_OFFSET
        );


    /*
     * EHCI BAR jest pamięciowy.
     *
     * bit 0 = 0
     */

    if (bar0 & 1) {

        usb_print(
            "EHCI: BAR0 is I/O, unsupported\n"
        );

        return -2;
    }


    /*
     * 32-bit BAR.
     *
     * Maskujemy flagi.
     */

    uint32_t mmio_base =
        bar0 & 0xFFFFFFF0;


    usb_print(
        "EHCI MMIO = "
    );

    usb_print_hex(mmio_base);

    usb_putc('\n');


    /*
     * Zakładamy identity mapping:
     *
     * physical == virtual
     */

    ehci_mmio =
        (volatile uint8_t*)mmio_base;


    /*
     * CAPLENGTH
     */

    uint8_t caplength =
        *(volatile uint8_t*)
            (ehci_mmio + EHCI_CAPLENGTH);


    ehci_operational =
        caplength;


    /*
     * HCSPARAMS
     */

    uint32_t hcsparams =
        ehci_read32(
            EHCI_HCSPARAMS
        );


    ehci_ports =
        hcsparams & 0x0F;


    usb_print(
        "EHCI ports = "
    );

    usb_print_dec(
        ehci_ports
    );

    usb_putc('\n');


    usb_print(
        "EHCI operational base = "
    );

    usb_print_hex(
        ehci_operational
    );

    usb_putc('\n');


    /*
     * Kontroler reset.
     */

    if (ehci_controller_reset() != 0)
        return -3;


    /*
     * Po resecie jeszcze raz
     * ustawiamy memory/bus master.
     */

    command =
        pci_read32(
            ehci_bus,
            ehci_slot,
            ehci_function,
            PCI_COMMAND_OFFSET
        );

    command |=
        PCI_COMMAND_MEMORY |
        PCI_COMMAND_BUSMASTER;

    pci_write32(
        ehci_bus,
        ehci_slot,
        ehci_function,
        PCI_COMMAND_OFFSET,
        command
    );


    /*
     * Porty.
     */

    usb_print(
        "Checking USB ports...\n"
    );


    for (uint8_t port = 0;
         port < ehci_ports;
         port++)
    {
        ehci_reset_port(port);
    }


    /*
     * Przygotuj QH.
     *
     * Na razie przykładowy:
     *
     * device = 1
     * endpoint = 1
     * max packet = 512
     *
     * Później wartości te będą pochodziły
     * z deskryptora endpointu.
     */

    ehci_qh_init(
        &ehci_bulk_qh,
        1,
        1,
        512
    );


    /*
     * Wyzeruj qTD.
     */

    for (int i = 0;
         i < EHCI_MAX_QTDS;
         i++)
    {
        ehci_qtds[i].next =
            EHCI_PTR_TERMINATE;

        ehci_qtds[i].alt_next =
            EHCI_PTR_TERMINATE;

        ehci_qtds[i].token = 0;
    }


    usb_print(
        "EHCI initialization OK\n"
    );


    return 0;
}


/* ============================================================
 * TEST BULK
 * ============================================================ */

/*
 * UWAGA:
 *
 * Ta funkcja jest tylko testem.
 *
 * Nie wywołuj jej, dopóki nie masz:
 *
 * device address
 * endpoint OUT
 * endpoint IN
 * max packet
 *
 * odczytanych podczas enumeracji USB.
 */

int usb_ehci_bulk_test(void)
{
    int out_toggle = 0;
    int in_toggle = 0;

    const char test[] =
        "LOTOS EHCI BULK TEST";


    uint8_t buffer[64]
        __attribute__((aligned(4096)));


    /*
     * Przykład:
     *
     * device 1
     * endpoint OUT 2
     * endpoint IN 1
     * max packet 512
     *
     * TE NUMERY SĄ TYLKO PRZYKŁADOWE.
     */


    usb_print(
        "EHCI BULK OUT test...\n"
    );


    int result =
        ehci_bulk_out(
            1,
            2,
            512,
            (void*)test,
            sizeof(test) - 1,
            &out_toggle
        );


    usb_print(
        "OUT result = "
    );

    usb_print_dec(
        (uint32_t)result
    );

    usb_putc('\n');


    /*
     * Bulk IN
     */

    for (int i = 0; i < 64; i++)
        buffer[i] = 0;


    usb_print(
        "EHCI BULK IN test...\n"
    );


    result =
        ehci_bulk_in(
            1,
            1,
            512,
            buffer,
            64,
            &in_toggle
        );


    usb_print(
        "IN result = "
    );

    usb_print_dec(
        (uint32_t)result
    );

    usb_putc('\n');


    return result;
}
