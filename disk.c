#include <stdint.h>

/*
 * LOTOS OS - disk.c
 * IDE/ATA PIO driver
 *
 * Obsługuje:
 *   - Primary IDE channel
 *   - Master / Slave
 *   - IDENTIFY
 *   - READ SECTORS (28-bit LBA)
 *   - WRITE SECTORS (28-bit LBA)
 *
 * Uwaga:
 * To jest sterownik IDE/ATA PIO.
 * Jeżeli laptop używa wyłącznie AHCI/SATA bez trybu IDE,
 * ten sterownik nie wystarczy.
 */

/* =========================
   I/O PORTS
   ========================= */

#define ATA_PRIMARY_DATA       0x1F0
#define ATA_PRIMARY_ERROR      0x1F1
#define ATA_PRIMARY_FEATURES   0x1F1
#define ATA_PRIMARY_SECCOUNT   0x1F2
#define ATA_PRIMARY_LBA_LOW    0x1F3
#define ATA_PRIMARY_LBA_MID    0x1F4
#define ATA_PRIMARY_LBA_HIGH   0x1F5
#define ATA_PRIMARY_DRIVE      0x1F6
#define ATA_PRIMARY_STATUS     0x1F7
#define ATA_PRIMARY_COMMAND    0x1F7
#define ATA_PRIMARY_CONTROL    0x3F6

/* =========================
   ATA COMMANDS
   ========================= */

#define ATA_CMD_READ_PIO       0x20
#define ATA_CMD_WRITE_PIO      0x30
#define ATA_CMD_IDENTIFY       0xEC
#define ATA_CMD_CACHE_FLUSH   0xE7

/* =========================
   STATUS BITS
   ========================= */

#define ATA_STATUS_ERR         0x01
#define ATA_STATUS_DRQ         0x08
#define ATA_STATUS_SRV         0x10
#define ATA_STATUS_DF          0x20
#define ATA_STATUS_RDY         0x40
#define ATA_STATUS_BSY         0x80

/* =========================
   DEVICE
   ========================= */

#define ATA_MASTER             0x00
#define ATA_SLAVE              0x10

static int disk_present = 0;
static int disk_slave = 0;

/* =========================
   PORT I/O
   ========================= */

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

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t value;

    __asm__ volatile (
        "inw %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}

static inline void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile (
        "outw %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

/* =========================
   WAIT 400ns
   ========================= */

static void ata_400ns_delay(void)
{
    inb(ATA_PRIMARY_STATUS);
    inb(ATA_PRIMARY_STATUS);
    inb(ATA_PRIMARY_STATUS);
    inb(ATA_PRIMARY_STATUS);
}

/* =========================
   WAIT UNTIL NOT BUSY
   ========================= */

static int ata_wait_bsy(void)
{
    uint32_t timeout = 1000000;

    while (timeout--)
    {
        uint8_t status = inb(ATA_PRIMARY_STATUS);

        if (!(status & ATA_STATUS_BSY))
            return 1;
    }

    return 0;
}

/* =========================
   WAIT FOR DRQ
   ========================= */

static int ata_wait_drq(void)
{
    uint32_t timeout = 1000000;

    while (timeout--)
    {
        uint8_t status = inb(ATA_PRIMARY_STATUS);

        if (status & ATA_STATUS_ERR)
            return 0;

        if (status & ATA_STATUS_DF)
            return 0;

        if (status & ATA_STATUS_DRQ)
            return 1;
    }

    return 0;
}

/* =========================
   IDENTIFY DISK
   ========================= */

int disk_identify(uint16_t *buffer)
{
    uint8_t status;

    /*
     * Wybierz master.
     */
    outb(
        ATA_PRIMARY_DRIVE,
        0xA0
    );

    ata_400ns_delay();

    /*
     * Wyzeruj parametry.
     */
    outb(ATA_PRIMARY_SECCOUNT, 0);
    outb(ATA_PRIMARY_LBA_LOW, 0);
    outb(ATA_PRIMARY_LBA_MID, 0);
    outb(ATA_PRIMARY_LBA_HIGH, 0);

    /*
     * IDENTIFY.
     */
    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_IDENTIFY
    );

    status = inb(ATA_PRIMARY_STATUS);

    /*
     * Brak urządzenia.
     */
    if (status == 0)
        return 0;

    /*
     * Czekamy aż BSY spadnie.
     */
    if (!ata_wait_bsy())
        return 0;

    /*
     * Czekamy na DRQ.
     */
    if (!ata_wait_drq())
        return 0;

    /*
     * Odczytujemy 256 słów = 512 bajtów.
     */
    for (int i = 0; i < 256; i++)
    {
        buffer[i] = inw(ATA_PRIMARY_DATA);
    }

    disk_present = 1;

    return 1;
}

/* =========================
   READ ONE SECTOR
   ========================= */

int disk_read_sector(
    uint32_t lba,
    uint8_t *buffer
)
{
    /*
     * ATA 28-bit LBA obsługuje:
     *
     * 0 ... 268435455
     */

    if (lba > 0x0FFFFFFF)
        return 0;

    if (!disk_present)
        return 0;

    /*
     * Wybór dysku + górne 4 bity LBA.
     */
    outb(
        ATA_PRIMARY_DRIVE,
        0xE0 | ((lba >> 24) & 0x0F)
    );

    /*
     * Jeden sektor.
     */
    outb(
        ATA_PRIMARY_SECCOUNT,
        1
    );

    /*
     * LBA.
     */
    outb(
        ATA_PRIMARY_LBA_LOW,
        lba & 0xFF
    );

    outb(
        ATA_PRIMARY_LBA_MID,
        (lba >> 8) & 0xFF
    );

    outb(
        ATA_PRIMARY_LBA_HIGH,
        (lba >> 16) & 0xFF
    );

    /*
     * READ SECTORS.
     */
    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_READ_PIO
    );

    /*
     * Czekamy.
     */
    if (!ata_wait_bsy())
        return 0;

    if (!ata_wait_drq())
        return 0;

    /*
     * 256 słów = 512 bajtów.
     */
    for (int i = 0; i < 256; i++)
    {
        uint16_t word = inw(ATA_PRIMARY_DATA);

        buffer[i * 2]     = word & 0xFF;
        buffer[i * 2 + 1] = (word >> 8) & 0xFF;
    }

    return 1;
}

/* =========================
   WRITE ONE SECTOR
   ========================= */

int disk_write_sector(
    uint32_t lba,
    const uint8_t *buffer
)
{
    if (lba > 0x0FFFFFFF)
        return 0;

    if (!disk_present)
        return 0;

    /*
     * Wybór dysku + LBA.
     */
    outb(
        ATA_PRIMARY_DRIVE,
        0xE0 | ((lba >> 24) & 0x0F)
    );

    /*
     * Jeden sektor.
     */
    outb(
        ATA_PRIMARY_SECCOUNT,
        1
    );

    /*
     * LBA.
     */
    outb(
        ATA_PRIMARY_LBA_LOW,
        lba & 0xFF
    );

    outb(
        ATA_PRIMARY_LBA_MID,
        (lba >> 8) & 0xFF
    );

    outb(
        ATA_PRIMARY_LBA_HIGH,
        (lba >> 16) & 0xFF
    );

    /*
     * WRITE SECTORS.
     */
    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_WRITE_PIO
    );

    /*
     * Czekamy na DRQ.
     */
    if (!ata_wait_bsy())
        return 0;

    if (!ata_wait_drq())
        return 0;

    /*
     * Wysyłamy 512 bajtów.
     */
    for (int i = 0; i < 256; i++)
    {
        uint16_t word =
            buffer[i * 2] |
            ((uint16_t)buffer[i * 2 + 1] << 8);

        outw(
            ATA_PRIMARY_DATA,
            word
        );
    }

    /*
     * Czekamy aż zapis się zakończy.
     */
    if (!ata_wait_bsy())
        return 0;

    /*
     * Flush cache.
     */
    outb(
        ATA_PRIMARY_COMMAND,
        ATA_CMD_CACHE_FLUSH
    );

    if (!ata_wait_bsy())
        return 0;

    return 1;
}

/* =========================
   DISK INIT
   ========================= */

int disk_init(void)
{
    uint16_t identify[256];

    disk_present = 0;

    /*
     * Spróbuj wykryć dysk.
     */
    if (!disk_identify(identify))
    {
        return 0;
    }

    return 1;
}

/* =========================
   STATUS
   ========================= */

int disk_is_present(void)
{
    return disk_present;
}
