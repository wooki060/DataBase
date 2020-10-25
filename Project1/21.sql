select T.name, count(C.owner_id)
from (
  select id, name
  from Trainer
  where id in (
    select leader_id
    from Gym
    )
  ) as T,
  CatchedPokemon as  C
where T.id = C.owner_id
group by C.owner_id
order by name
