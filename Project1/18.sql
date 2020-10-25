select avg(level)
from CatchedPokemon as C, Gym as G
where C.owner_id = G.leader_id
