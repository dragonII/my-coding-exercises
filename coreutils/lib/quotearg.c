struct quoting_options
{
    /* Basic quoting style */
    enum quoting_style style;

    /* Addtional flags. Bitwise combination of enum quoting_flags */
    int flags;

    /* Quote the characters indicated by this bit vector even if the
       quoting style would not normally require them to be quoted */
    unsigned int quote_these_too[(UCHAR_MAX / INT_BITS) + 1];

    /* The left quote for custom_quoting_style */
    char const* left_quote;

    /* The right quote for custom_quoting_style */
    char const* right_quote;
};

char* quotearg_char_mem(char const* arg, size_t argsize, char ch)
{
    struct quoting_options options;
    options = default_quoting_options;
    set_char_quoting(&options, ch, 1);
    return quotearg_n_options(0, arg, argsize, &options);
}

char* quotearg_char(char const* arg, char ch)
{
    return quotearg_char_mem(arg, SIZE_MAX, ch);
}

char* quotearg_colon(char const* arg)
{
    return quotearg_char(arg, ':');
}
