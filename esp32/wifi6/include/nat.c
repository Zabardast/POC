#include "lwip/ip_addr.h"

#define MAX_NAT_ENTRIES 2000 //65535
static int current_entries = 0;

typedef struct sta_address {
    ip4_addr_t ip;
    int port;
}sta_address_t;

// struct ap_address {
//     ip4_addr_t ip;
//     int port;
// };

typedef struct ap_public_address {
    ip4_addr_t ip;
    int port;
}ap_public_address_t;

typedef struct target_address {
    ip4_addr_t ip;
    int port;
} target_address_t; 

// builder
struct sta_address sta_address()
{
    sta_address_t sta = { .ip.addr = 0, .port = 0};

    return sta;
}

struct ap_public_address ap_public_address()
{
    ap_public_address_t appa = { .ip.addr = 0, .port = 0};
    return appa;
}

//{sta} -(target)-> [ap] || [ap public] -> [ target ]

typedef struct Translation_entry{
    struct sta_address sta_address;
    // struct ap_address ap_address; // not needed because port is target_address prot and address is local
    struct ap_public_address ap_public_address;
    struct target_address target_address; // I maybe dont need this
    int protocol;
    struct Translation_entry *next;
} Translation_entry_t;

    // need to sotre the table in a dynamic table.
// static struct Translation_entry translation_table[MAX_NAT_ENTRIES];
Translation_entry_t * translation_table = NULL; // setup in NAT_task
Translation_entry_t * end_translation_table = NULL;

int add_translation(struct sta_address p_sta_address, struct ap_public_address p_ap_public_address, int p_protocol)
{
    if(current_entries >= MAX_NAT_ENTRIES) return -1;

    end_translation_table->next = (Translation_entry_t *) malloc(sizeof(Translation_entry_t));

    end_translation_table->next->sta_address = p_sta_address;
    end_translation_table->next->ap_public_address = p_ap_public_address;
    end_translation_table->next->protocol = p_protocol;
    end_translation_table->next->next = NULL;

    end_translation_table = end_translation_table->next; 

    current_entries++;

    return 0;
}

// TODO: the parameters are bigger than they should be
int remove_translation(target_address_t p_target, ap_public_address_t port) // public port and target ip
{
    for(Translation_entry_t * tabl = translation_table; tabl != end_translation_table; tabl = tabl->next)
    {
        if (tabl->next == NULL)
            return -1; // didn't find

        if(tabl->next->target_address.ip.addr == p_target.ip.addr && tabl->ap_public_address.port == port.port)
        {
            Translation_entry_t * tmp = tabl->next;
            tabl->next = tabl->next->next;
            free(tmp);
            return 0;
            current_entries--;
        }
    }

    return -1; // TODO: should handle this as an error ?
}



// for incoming messages we need to know where to send them.  // you are looking for the local ip address and port
struct sta_address find_sta_translation(int ap_public_port)
{ // TODO: optimize this for search time
    // for(int index = 0; index < MAX_NAT_ENTRIES; index++)
    for(Translation_entry_t * tabl = translation_table; tabl != end_translation_table; tabl = tabl->next)
    {
        if(tabl->ap_public_address.port == ap_public_port)
            return tabl->sta_address;
    }
    return sta_address(); // should be changed to represent an error
}


// for outgoing meessages only  // you are looking for the public ap port
int find_ap_translation(struct sta_address sta_address)
{
    for(Translation_entry_t * tabl = translation_table; tabl != end_translation_table; tabl = tabl->next)
    {
        if(tabl->sta_address.port == sta_address.port)
            return tabl->ap_public_address.port;
    }
    return -1; // probably loop back and add a new translation entry
}


void NAT_task()
{
    //setup

    translation_table = (Translation_entry_t *) malloc(sizeof(Translation_entry_t));

    end_translation_table = translation_table;

    end_translation_table->next = NULL;
    end_translation_table->ap_public_address = ap_public_address();
    end_translation_table->sta_address = sta_address();
    end_translation_table->protocol = 0;

    // do the header translation
    for(;;)
    {
        // get packet
        // esp_wifi_set_promiscuous_rx_cb();
        

        // make new header
        // send packet
    }
}