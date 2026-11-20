# LOVE EQ — RELATIONSHIP EQ, première intégration

## Objectif
Permettre à plusieurs pistes de comparer leur énergie spectrale sans mélanger leurs signaux audio. L'utilisateur choisit le rôle de chaque instance et la piste prioritaire. Une correction dynamique limitée peut être activée explicitement sur la piste moins prioritaire.

## Parcours utilisateur
1. Insérer LOVE EQ sur la voix et sur la guitare ou la basse.
2. Dans les instances concernées, ouvrir RELATION EQ dans la barre supérieure.
3. Choisir **RÔLE** : Voix, Basse, Guitare ou Autre. Choisir le même **GROUPE** (1 à 4).
4. Sur l'instance à corriger, choisir **PRIORITÉ** (par exemple Voix sur la guitare).
5. Mettre les pistes en lecture et cliquer sur **PROPOSER**.
6. LOVE EQ mesure le chevauchement de 12 bandes spectrales réelles. Harmonic DNA fournit une fondamentale lorsque le signal est suffisamment monophonique. La proposition évite les premières harmoniques proches de cette fondamentale.
7. **ACTIVER** est disponible seulement lorsqu'une proposition a été calculée et qu'une fondamentale exploitable a été identifiée. Le clic applique un filtre Bell dynamique (Q 1,4, réduction maximale de 3 dB). **ARRÊTER** désactive la correction.

## Moteur implémenté
- Échanges entre instances du même plugin et du même processus hôte par un registre statique de 32 emplacements atomiques : aucun mutex, allocation ni accès disque dans le thread audio.
- Quatre groupes, quatre rôles musicaux plus « Non affecté », association explicite. Un instrument prioritaire ne se corrige jamais lui-même.
- 12 bandes fréquentielles mesurées en temps réel par un filtre passe-bas différentiel, à faible coût, une mesure RMS par bloc.
- Les données de plusieurs instances avec le même rôle sont agrégées par maximum de niveau dans chaque bande.
- Fenêtre d'expiration des données d'une instance qui ne reçoit plus de blocs audio, évitant de conserver une activité ancienne.
- Détection de chevauchement vérifiant le niveau relatif de la piste et de la source prioritaire, plutôt qu'une égalisation permanente.
- Filtre paramétrique additionnel dont la réduction est pilotée par les niveaux partagés : retour progressif vers zéro quand la piste prioritaire ou la piste cible n'est plus active.
- Attaque de 35 ms, relâchement de 240 ms et réduction limitée à 0…-3 dB lors de l'activation (plafond de paramètre 6 dB).
- Protection de la fondamentale et des six premiers harmoniques par exclusion des bandes candidates proches, et blocage du filtre si la fréquence choisie est protégée par la fondamentale sauvegardée.
- Paramètres de rôle, groupe, priorité, activité, fréquence, Q, intensité et fondamentale protégée dans l'APVTS ; automatisation et restauration par la session hôte.
- La version active ne réutilise ni le DSP d'EQ Match ni des animations simulées. La détection est indépendante de l'interface : le traitement continue lorsque celle-ci est fermée.

## Limites de cette première phase
- Le registre ne communique **qu'entre instances de la même bibliothèque binaire au sein du même processus**. Des plugins isolés dans différents processus, ou chargés à la fois en AU et VST3, ne partagent pas ces données.
- Il n'y a pas encore de nommage automatique des pistes du DAW, de communication réseau ni de moteur global multihôte.
- Les 12 bandes sont des estimations perceptives grossières et non une analyse FFT détaillée des instances distantes. Les futurs versions pourront enrichir la résolution et intégrer un sidechain de secours.
- Le système de protection harmonique dépend d'une estimation monophonique ; un accord, un mix complet ou une source percussive peuvent laisser la correction indisponible (protection prudente).
- La protection mémorisée correspond à la fondamentale validée au moment de l'activation. Les variations mélodiques rapides nécessitent une version future de suivi continu indépendant de l'éditeur.
- La correction est dynamique en niveau dans le temps, mais ne reconnaît pas encore les attaques, les syllabes ni les différentes notes simultanées.
- Aucune correction n'est activée par défaut ; pas de perte de signal sur une source silencieuse ni de coupe sur un harmonique clairement identifié.

## Contrôles qualité
Tests Linux et macOS : mappage spectral, mesure de la présence audio, isolation des groupes, agrégation entre instances, exclusion des harmoniques, calcul de la réduction dynamique et expiration des sources anciennes. Compilation AU, VST3 et Standalone, puis artefacts uniquement après build vert.

## Interface
Conserver l'apparence bleu nuit Liberty, le français par défaut avec accents UTF-8, les menus professionnels, la fenêtre 1160 × 760 sans ascenseur et aucun chevauchement. RELATION EQ remplace Harmonic DNA seulement dans le troisième panneau inférieur, sans retirer les fonctions déjà livrées.
