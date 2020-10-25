select count(distinct type)
from Pokemon
where id in (
  select pid
  from CatchedPokemon
  where owner_id in (
    select leader_id
    from Gym
    where city = 'Sangnok City'
    )
  )
