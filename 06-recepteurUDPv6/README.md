# Algorithmes des réseaux

## Récepteur UDP IPv6

Complétez le programme `receiver-udp.c` pour recevoir un message texte avec les protocoles `IPv6` et `UDP` et l'afficher sur la sortie standard. Vous devrez également afficher l'adresse IP et le port de l'expéditeur du message.

Le programme admet en argument le numéro de port sur lequel le programme doit écouter :

    ./receiver-udp port_number

Les seuls numéros de port valides sont ceux contenus dans l'intervalle `[10000; 65000]`.

La sortie de votre programme doit être de la forme suivante :

    ./receiver-udp 10001
    hello world
    2001:db8:dead:beef::1 57123

Une telle sortie indique que votre programme a reçu le message `hello world` d'une source dont l'adresse IP est `2001:db8:dead:beef::1` et le port est `57123`.

On fera l'hypothèse que le message aura une taille maximum de `SIZE` caractères avec `SIZE < 100`.

**Objectifs :** savoir recevoir un message texte en UDP.

## Marche à suivre

Vous pouvez directement reprendre le code réalisé dans l'exercice 4 et mettre à jour les parties spécifiques à `IPv4`. Ici le programme devra écouté sur l'adresse `::1` et le port passé en argument du programme.

Vous pouvez tester votre programme avec le programme réalisé dans l'exercice 5 ou en exécutant la commande `nc` (netcat) dans un autre terminal. Lancez votre programme dans un terminal puis exécutez la commande suivante dans un second terminal afin d'envoyer le message `hello world` en `UDP` sur l'adresse `::1` et le port utilisé par votre programme, et en utilisant le port local de votre choix :

    echo "hello world" | nc -6u -w1 ::1 port_number -p port_local_pour_netcat

Vérifiez ensuite que le message ainsi que l'adresse IP et le port source sont correctement affichés par votre programme sur la sortie standard.

## Validation

Votre programme doit obligatoirement passer tous les tests sur gitlab (il suffit de `commit/push` le fichier source pour déclencher le pipeline de compilation et de tests) avant de passer à l'exercice suivant.
