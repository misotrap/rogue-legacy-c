# Audit C++ / C# (29/10/2026)

Quatre audits en lecture seule (héros, ennemis, château et physique, écrans et progression), classés du plus important au moins important. Les doublons entre audits sont fusionnés. Chemins C# sous `ref/RogueLegacy1/RogueCastle/src/`, chemins C++ sous `src/RogueCastle/`.

## A. Critique (le jeu ne se joue pas comme l'original)

1. **Les ennemis tués réapparaissent** à chaque retour dans une salle. `EnemyObj::ResetState` remet `IsKilled = false` alors que la version C# ne le fait jamais (EnemyObj.cs:1113). De plus, le portage réinitialise aussi la salle d'arrivée.
2. **Pas de niveau d'ennemi.**
   - Il manque Level = niveau de la salle / ROOM_LEVEL_MOD, la promotion de difficulté, et les bonus des orbes jaunes et des mini-boss (ProceduralLevelScreen.cs:544-604).
   - Il manque aussi le gain de PV, dégâts et XP par niveau (EnemyObj.cs:1222).
   - Le terme de New Game+ et les runes de niveau de salle sont absents (ProceduralLevelScreen.cs:860, 885).
3. **L'armure ne réduit pas les dégâts reçus** (PlayerObj.cs:2296, réduction = armure/(ARMOR_DIVIDER+armure)).
4. **Le héros garde le contrôle pendant qu'il est blessé**, alors qu'en C# les commandes sont ignorées tant qu'il est en état HURT (PlayerObj.cs:575). Le recul part du mauvais côté : le C# éloigne le héros du centre de l'attaquant (PlayerObj.cs:2360). À l'atterrissage, il manque aussi le démarrage de l'invincibilité, l'annulation de l'attaque en l'air et la remise à zéro de AccelerationX.
5. **Les projectiles ennemis traversent les murs et n'expirent jamais.** En C#, les valeurs par défaut sont CollidesWithTerrain, DestroysWithTerrain et Lifespan 10 (ProjectileManager.cs:402). Sont concernés : chevalier, chevalier à l'épée, ninja, plante, archer, Starburst et les sorts des mages. De plus, l'épée et les sorts du joueur détruisent ces projectiles, alors qu'en C# seul le corps du joueur le fait (PlayerObj.cs:2050).
6. **Les pièges (HazardObj, sols à pics) ne sont pas portés** : il y en a 285 dans les salles (LevelParser.cs:98, HazardObj.cs).
7. **Le butin tombe à travers le sol**, faute de boîte de terrain (ItemDropObj.cs:87, 162). À vérifier en jeu.
8. **Les conditions des coffres de fée sont testées pour tous les coffres du château**, et non pour la seule salle courante (FairyChestObj.cs:140).
9. **Rien à la mort d'un ennemi** (EnemyObj.cs:884-1010) :
   - pas de vampirisme ;
   - pas de gain de mana : mage, compétence et runes ;
   - pas de PV max pour la Liche ;
   - pas de pièces de l'objet GoldPerKill ;
   - pas de compteurs.
10. **Pas de coups critiques** (EnemyObj.cs:675, PlayerObj.cs:3951). Il manque aussi la mana de l'Épée-sort au contact.
11. **Les runes et l'équipement de double saut, d'esquive en l'air et de vol ne marchent pas** (PlayerObj.cs:4030). Le vol est entièrement absent. Le compteur d'esquives en l'air se remet à zéro à la fin de l'esquive, au lieu de l'atterrissage.
12. **L'entrée du château ne se ferme pas** derrière le héros : il peut revenir dans la cour en pleine partie (CastleEntranceRoomObj.cs:221, 276).
13. **Continuer une partie remet le héros dans la cour, avec Charon.** Le C# reprend à l'entrée du château, avec LoadStartingRoom (TitleScreen.cs:351, LoadingScreen.cs:277).
14. **L'architecte verrouille le mauvais château** : la taxe est payée sur un château neuf, et l'héritier suivant profite du château verrouillé gratuitement (StartingRoomObj.cs:733, LoadingScreen.cs:234).
15. **Lecture hors limites (plantage possible)** : le squelette Advanced appelle `PickAction(w, 7, acts)` alors que `acts` n'a que 6 entrées (EnemyObj_Skeleton.cpp:326).

## B. Important

16. **Les attaques vers le bas font le double des dégâts.** Il manque le facteur 0,5, le bonus Down_Strike_Up et le compteur NumAirBounces, ainsi que le rebond sur les objets cassables (PlayerObj.cs:3568, 1949).
17. **L'épée repousse peu** : le C# utilise StrongEnemyKnockBack (300,360) au sol, le portage (90,90) partout (PlayerObj.cs:3609).
18. **Multiplicateurs manquants** : les PV et la mana du Dragon et du Traître, et LichHealthMod (PlayerObj.cs:4207, 4243, 3684). Le Liche2 ne peut jamais convertir ses PV en mana.
19. **Le Dragon se joue comme un chevalier** : pas de vol, l'attaque ne lance pas de sort, pas de régénération (PlayerObj.cs:847, 1002, 1056). Il manque aussi le Tanuki du Chevalier2, la lampe du Banquier2, le bouclier « Close » du Traître, et ses haches et dagues (mauvaises valeurs).
20. **La vitesse de déplacement ignore** le Ninja (+0,3), l'équipement et les runes (PlayerObj.cs:4055).
21. **Mort** : il manque l'objet de résurrection, l'esquive de la mort (Death_Dodge), la perte de pièces, le renvoi de dégâts et l'enregistrement du tueur (PlayerObj.cs:2300-2432). La Liche garde ses bonus d'un héritier à l'autre (GameOverScreen.cs:177).
22. **Compétences sans effet** : Invuln_Time_Up, Attack_Speed_Up, coût du blocage. Le blocage ne fait ni recul ni clignotement (PlayerObj.cs:3697, 246, 3940, 2065).
23. **La vie et la mana ne sont pas bornées** : les potions soignent au-delà du maximum, la mana peut devenir négative (PlayerObj.cs:47, CharacterObj.cs:49).
24. **L'arrêt du temps ne fige pas les ennemis** (PauseEnemy) : physique, recul et projectiles continuent (ProceduralLevelScreen.cs:2563).
25. **Temps de repos des ennemis faux** : le C# a un bloc de repos fait de marches. Le portage met la vitesse à 0 à la fin de chaque action, ce qui fait s'arrêter net le loup (EnemyObj.cs:293-347).
26. **Mécaniques d'ennemis absentes** :
    - le Blob ne se divise pas (et le boss du donjon meurt au premier coup) ;
    - l'Énergon n'a pas son bouclier ni ses projectiles à renvoyer ;
    - le chevalier à la boule ne laisse pas de boules rebondissantes à sa mort, et sa boule reste figée ;
    - le Mimic attaque sans être provoqué ;
    - le Ninja se téléporte, ce qu'il ne fait jamais dans l'original ;
    - les mages ne retirent pas leur sort quand ils meurent.
27. **Variantes expert et mini-boss simplifiées** : chevalier, squelette (rage), Starburst, boule de feu, invocations de la fée, bouclier de feu du cheval, Blob expert, archer (temps de repos en trop).
28. **Rebords** : le C# arrête l'ennemi au bord, le portage le fait demi-tourner. Le C# arrête aussi les ennemis contre les murs (EnemyObj.cs:756).
29. **Les ennemis volants n'appliquent pas leur accélération**, donc pas de recul (PhysicsManager.cs:91).
30. **Projectiles** :
    - pas de poursuite pour la tourelle experte ni pour l'Énergon ;
    - pas de rotation des flèches (FollowArc) ;
    - collision avec les plateformes à sens unique et avec les portes ;
    - nettoyage hors salle et au changement de salle différent du C# (le butin n'est pas effacé).
31. **Le butin ignore le bonus d'or** : compétence, équipement, runes, New Game+ et Banquier (ItemDropObj.cs:104).
32. **Le coffre du boss s'ouvre tout seul** : dans l'original, il tombe, puis c'est le joueur qui l'ouvre (BossRoomObj.cs:134).
33. **Téléporteurs et carte** : pas de téléporteurs, pas de légende ni d'icônes, et les salles visitées ne sont pas sauvegardées.
34. **Progression** :
    - le manoir ne s'ouvre pas après le choix de l'héritier ;
    - l'amélioration de classe n'est pas appliquée au héros en cours, et l'écran de déblocage manque ;
    - pas de plans ni de runes de départ ;
    - Johannes n'est pas un vrai ancêtre ;
    - la mort renvoie au lignage au lieu de l'écran titre ;
    - pas de confirmation du choix de l'héritier ;
    - la liste de sorts du Mage2 manque (WizardSpellList) ;
    - le drapeau Prosopagnosie n'est jamais posé.
35. **Écrans** :
    - mort : le tueur n'est pas nommé et le défilé des ennemis tués manque ;
    - pause : mauvais niveaux d'ennemis (×2,75), pièges listés, magie et armure absentes ;
    - forgeron et enchanteresse : acheter n'équipe pas ;
    - options : il manque QuickDrop, Traître et les confirmations ;
    - journal de l'entrée du château absent ;
    - suffixe « + » et couronne du titre absents.

## C. Mineur

- **Butin et décor** :
  - le bonus de mana max ne remplit pas la mana actuelle ;
  - la phobie des poulets ne fait pas apparaître de poulet ;
  - les rochers du jardin ne flottent pas ;
  - la caméra n'est pas arrondie au pixel ;
  - la démence peut copier une plateforme.
- **Coffres** : GivePrint donne de l'or au lieu de stats sur un coffre d'or.
- **Salles** : pas de délai de 0,2 s à l'entrée, et les ennemis ne sont pas posés au sol au chargement.
- **Héros** :
  - pas d'état JUMPING en tombant d'un rebord ;
  - le héros peut se retourner pendant une attaque ;
  - l'attaque n'annule pas l'invisibilité de l'Assassin ;
  - le Savant tire un nouveau sort mais paie le coût de l'ancien ;
  - RoomTransitionReset est vide (la balise de translocation reste d'une salle à l'autre) ;
  - le Mage2 fait tourner les 8 sorts au lieu de sa liste de 3.
- **Interface** : barres de vie et de mana qui ne s'allongent pas, et icônes du sort, de l'objet et des runes absentes.
- **Manoir** : la vie et la mana ne sont pas remplies en sortant.

## Conformes

- Constantes PlayerEV et GameEV.
- Génération du château (LevelBuilder2).
- Coûts : compétences, runes et plans.
- Génération des héritiers.
- Péage de Charon.
- Ennemis : Eyeball, plante, zombie et Spark.

Non vérifiés en détail : LastBoss, SwordKnight, Chicken, BouncySpike, SpikeTrap, sorts de l'EarthWizard.
