int isupper(c)
        char c;
{
        return((c >= 'A') && (c <= 'Z'));
}

int islower(c)
        char c;
{
        return((c >= 'a') && (c <= 'z'));
}

int isletter(c)
        char c;
{
        return(isupper(c) || islower(c));
}
