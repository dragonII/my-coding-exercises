/* Demo for chained hash table */

struct CHTable_
{
    int     n_buckets;       /* Maximum number of the buckets allocated */
    int     (*match)(void* key1, void* key2);
    void    (*destroy)(void* data);
    int     current_size;
    List*   table;           /* Array of buckets */
};

typedef struct CHTable_ CHTable_t;

/* Hash table initialization.
   Return 0 when successful, -1 otherwise */

int chtb_init(CHTable_t* htb, int n_buckets, int (*h)(void* key),
                int (*match)(void* key1, void* key2),
                void (*destroy)(void* data))
{
    int i;

    if((htb->table = (List*)malloc(n_buckets * sizeof(List))) == NULL)
        return -1;

    htb->n_buckets = n_buckets;

    for(i = 0; i < htb->n_buckets; i++)
        list_init(&htb->table[i], destroy);

    htb->h = h;
    htb->match = match;
    htb->destroy = destroy;

    htb->current_size = 0;

    return 0;
}

/* destroy any dynamically allocation data */
void chtb_destroy(CHTable_t* htb)
{
    int i;

    for(i = 0; i < htb->n_buckets; i++)
    {
        list_destroy(&htb->table[i]);
    }

    free(htb->table);

    memset(htb, 0, sizeof(CHTable_t));
    return;
}

/* Return 0 if inserting successfully, 1 when element is already
   in the hash table, -1 when failed */
int chtb_insert(CHTable_t* htb, void* data)
{
    void* temp;
    int bucket, retval;

    temp = (void*)data;
    if(chtb_lookup(htb, &temp) == 0)
        return 1;

    bucket = htb->h(data) % htb->n_buckets;

    if((retval = list_ins_next(&htb->table[bucket], NULL, data)) == 0)
        htb->current_size++;

    return retval;
}

/* Return 0 if removing successfully, -1 otherwise */
int chtb_remove(CHTable_t* htb, void** data)
{
    ListElmt *element, *prev;
    int bucket;

    bucket = htb->h(*data) % htb->n_buckets;

    prev = NULL;

    for(element = list_head(&htb->table[bucket]); element != NULL;
            element = list_next(element))
    {
        if(htb->match(*data, list_data(element)))
        {
            if(list_rem_next(&htb->table[bucket], prev, data) == 0)
            {
                htb->current_size--;
                return 0;
            }
            else
                return -1;
        }
        prev = element;
    }
    return -1;
}

/* Return 0 if found in the hash table, -1 otherwise */
int chtb_lookup(CHTable_t* htb, void** data)
{
    ListElmt* element;
    int bucket;

    bucket = htb->h(*data) % htb->n_buckets;

    for(element = list_head(&htb->table[bucket]); element != NULL;
            element = list_next(element))
    {
        if(htb->match(*data, list_data(element)))
        {
            *data = list_data(element);
            return 0;
        }
    }
    return -1;
}


/* Return the number of elements in the hash table */
int chtb_size(CHTable_t* htb)
{
    return htb->current_size;
}
