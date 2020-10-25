select owner_id, count(*)
from CatchedPokemon
group by owner_id
having count(owner_id) >= (select count(owner_id)
                           from CatchedPokemon
                           group by owner_id
                           order by count(owner_id) desc
                           limit 1)
order by owner_id
