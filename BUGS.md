# Bugs signalés (test du 28/10/2026)

- [x] Coffres qui s'ouvrent à l'infini (SpriteObj::CurrentFrame comptait depuis 0)
- [x] Sol invisible : les colonnes BGLayer étaient dessinées devant les murs
- [x] Portraits : tableau dans le cadre, tremblement, poursuite en tournant, tirs (à vérifier en jeu)
- [x] Canon : tirait du mauvais côté une fois retourné vers le joueur
- [x] Lustres posés par terre (Weighted=False ignoré)
- [x] Objets cassables (chandeliers, chaises...) : les morceaux s'envolent et disparaissent (tweens de Break)
- [x] Étagères : plateformes (PhysicsObj Collidable, dessus seulement)
- [x] Rampes : pentes à 45° (CollHullObj tournés), le héros repose sur son coin côté montée
- [x] Saut variable (JumpDeceleration 5000 quand on relâche)
- [x] Bas + saut sur une plateforme : descendre à travers
- [x] Bas + attaque en l'air : attaque vers le bas (sinon coup normal)
- [x] Ennemis « sans dégâts » : c'était l'invincibilité après un coup, maintenant visible (héros transparent)
- [x] Après un coup, le héros glisse lentement sur le côté (même cause)
- [x] Le héros dérivait : pas de freinage de l'élan vers la droite
- [x] Mana de départ : chaque héritier commence avec vie et mana au maximum
- [x] Animation de mort : noir, le héros glisse au centre sous le projecteur, animation de mort, fantôme
- [x] Lancer d'os : les projectiles lestés (os, flèches, crachats) n'avaient pas de vitesse de départ ; ils tournent aussi
