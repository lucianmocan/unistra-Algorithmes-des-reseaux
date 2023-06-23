# Algorithmes des réseaux

## Expéditeur UDP IPv6

Complétez le programme `sender-udp.c` pour envoyer le message texte `hello world` avec les protocoles `IPv6` et `UDP`.

Le programme admet en argument le numéro de port de l'hôte distant à contacter :

    ./sender-udp port_number

Les seuls numéros de port valides sont ceux contenus dans l'intervalle `[10000; 65000]`.

**Objectifs :** savoir créer un socket IPv6 et transmettre un message texte en UDP.

## Marche à suivre :

Vous pouvez directement reprendre le code réalisé dans l'exercice 2 et mettre à jour les parties spécifiques à `IPv4`. Ici l'hôte distant sera joignable sur l'adresse `::1` et le port passé en argument du programme.

À titre d'information, le type `struct sockaddr_in6` défini dans `<netinet/in.h>` correspond à :

    struct sockaddr_in6 {
        sa_family_t     sin6_family;   /* famille                                   */
        in_port_t       sin6_port;     /* port au format réseau                     */
        uint32_t        sin6_flowinfo; /* IPv6 flow information                     */
        struct in6_addr sin6_addr;     /* adresse IPv6, cf. ci-dessous              */
        uint32_t        sin6_scope_id; /* index d'interface pour adresse lien-local */
    };

    struct in6_addr {
        unsigned char   s6_addr[16];   /* adresse IPv6 au format réseau */
    };

Néanmoins, on rappelle de toujours passer par une structure `struct sockaddr_storage` et de caster ensuite cette dernière afin d'écrire des programmes les plus indépendants possibles des familles des sockets :

    struct sockaddr_storage ss;
    struct sockaddr_in6 *in6 = (struct sockaddr_in6 *) &ss;

Vous pouvez tester votre programme en exécutant la commande `nc` (netcat) dans un autre terminal afin d'écouter en UDP sur l'adresse `::1` et le port de votre choix :

    nc -6ul ::1 port_number

Vous pouvez ensuite lancer votre programme dans un autre terminal et vérifier que le message transmis est bien affiché par `netcat` sur la sortie standard.

## Validation

Votre programme doit obligatoirement passer tous les tests sur gitlab (il suffit de `commit/push` le fichier source pour déclencher le pipeline de compilation et de tests) avant de passer à l'exercice suivant.
